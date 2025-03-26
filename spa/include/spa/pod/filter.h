/* Simple Plugin API */
/* SPDX-FileCopyrightText: Copyright © 2018 Wim Taymans */
/* SPDX-License-Identifier: MIT */

#ifndef SPA_POD_FILTER_H
#define SPA_POD_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <spa/param/props.h>
#include <spa/pod/iter.h>
#include <spa/pod/builder.h>
#include <spa/pod/compare.h>

#ifndef SPA_API_POD_FILTER
 #ifdef SPA_API_IMPL
  #define SPA_API_POD_FILTER SPA_API_IMPL
 #else
  #define SPA_API_POD_FILTER static inline
 #endif
#endif

/**
 * \addtogroup spa_pod
 * \{
 */

SPA_API_POD_FILTER int spa_pod_choice_fix_default(struct spa_pod_choice *choice)
{
	void *val, *alt;
	int i, nvals;
	uint32_t type, size;

	nvals = SPA_POD_CHOICE_N_VALUES(choice);
	type = SPA_POD_CHOICE_VALUE_TYPE(choice);
	size = SPA_POD_CHOICE_VALUE_SIZE(choice);
	alt = val = SPA_POD_CHOICE_VALUES(choice);

	switch (choice->body.type) {
	case SPA_CHOICE_None:
		break;
	case SPA_CHOICE_Range:
	case SPA_CHOICE_Step:
		if (nvals > 1) {
			alt = SPA_PTROFF(alt, size, void);
			if (spa_pod_compare_value(type, val, alt, size) < 0)
				memcpy(val, alt, size);
		}
		if (nvals > 2) {
			alt = SPA_PTROFF(alt, size, void);
			if (spa_pod_compare_value(type, val, alt, size) > 0)
				memcpy(val, alt, size);
		}
		break;
	case SPA_CHOICE_Flags:
	case SPA_CHOICE_Enum:
	{
		void *best = NULL;

		for (i = 1; i < nvals; i++) {
			alt = SPA_PTROFF(alt, size, void);
			if (spa_pod_compare_value(type, val, alt, size) == 0) {
				best = alt;
				break;
			}
			if (best == NULL)
				best = alt;
		}
		if (best)
			memcpy(val, best, size);

		if (nvals <= 1)
			choice->body.type = SPA_CHOICE_None;
		break;
	}
	}
	return 0;
}

SPA_API_POD_FILTER int spa_pod_filter_flags_value(struct spa_pod_builder *b,
		uint32_t type, const void *r1, const void *r2, uint32_t size SPA_UNUSED)
{
	switch (type) {
	case SPA_TYPE_Int:
	{
		int32_t val = (*(int32_t *) r1) & (*(int32_t *) r2);
		if (val == 0)
			return 0;
		spa_pod_builder_int(b, val);
		break;
	}
	case SPA_TYPE_Long:
	{
		int64_t val = (*(int64_t *) r1) & (*(int64_t *) r2);
		if (val == 0)
			return 0;
		spa_pod_builder_long(b, val);
		break;
	}
	default:
		return -ENOTSUP;
	}
	return 1;
}

SPA_API_POD_FILTER int spa_pod_filter_is_step_of(uint32_t type, const void *r1,
		const void *r2, uint32_t size SPA_UNUSED)
{
	switch (type) {
	case SPA_TYPE_Int:
		return *(int32_t *) r1 % *(int32_t *) r2 == 0;
	case SPA_TYPE_Long:
		return *(int64_t *) r1 % *(int64_t *) r2 == 0;
	case SPA_TYPE_Rectangle:
	{
		const struct spa_rectangle *rec1 = (struct spa_rectangle *) r1,
		    *rec2 = (struct spa_rectangle *) r2;

		return (rec1->width % rec2->width == 0 &&
		    rec1->height % rec2->height == 0);
	}
	default:
		return -ENOTSUP;
	}
	return 0;
}

SPA_API_POD_FILTER int spa_pod_filter_is_in_range(uint32_t type, const void *v,
		const void *min, const void *max, const void *step, uint32_t size SPA_UNUSED)
{
	if (spa_pod_compare_value(type, v, min, size) < 0 ||
	    spa_pod_compare_value(type, v, max, size) > 0)
		return 0;
	if (step != NULL)
		return spa_pod_filter_is_step_of(type, v, step, size);
	return 1;
}

SPA_API_POD_FILTER int
spa_pod_filter_prop(struct spa_pod_builder *b,
	    const struct spa_pod_prop *p1,
	    const struct spa_pod_prop *p2)
{
	const struct spa_pod *v1, *v2;
	struct spa_pod_choice *nc;
	uint32_t j, k, nalt1, nalt2;
	void *alt1, *alt2, *a1, *a2;
	uint32_t type, size, p1c, p2c;
	struct spa_pod_frame f;
	int res, n_copied = 0;

	v1 = spa_pod_get_values(&p1->value, &nalt1, &p1c);
	alt1 = SPA_POD_BODY(v1);
	v2 = spa_pod_get_values(&p2->value, &nalt2, &p2c);
	alt2 = SPA_POD_BODY(v2);

	type = v1->type;
	size = v1->size;

	/* incompatible property types */
	if (type != v2->type || size != v2->size || p1->key != p2->key)
		return -EINVAL;

	/* start with copying the property */
	spa_pod_builder_prop(b, p1->key, p1->flags & p2->flags);
	spa_pod_builder_push_choice(b, &f, 0, 0);
	nc = (struct spa_pod_choice*)spa_pod_builder_frame(b, &f);

	/* we should prefer alt2 values but only if they are within the
	 * range, start with an empty child and we will select a good default
	 * below */
	spa_pod_builder_child(b, size, type);

	if ((p1c == SPA_CHOICE_None && p2c == SPA_CHOICE_None) ||
	    (p1c == SPA_CHOICE_None && p2c == SPA_CHOICE_Enum) ||
	    (p1c == SPA_CHOICE_Enum && p2c == SPA_CHOICE_None) ||
	    (p1c == SPA_CHOICE_Enum && p2c == SPA_CHOICE_Enum)) {
		/* copy all equal values. Start with alt2 so that they are prefered.  */
		for (j = 0, a2 = alt2; j < nalt2; j++, a2 = SPA_PTROFF(a2, size, void)) {
			for (k = 0, a1 = alt1; k < nalt1; k++, a1 = SPA_PTROFF(a1,size,void)) {
				if (spa_pod_compare_value(type, a1, a2, size) == 0) {
					if (n_copied++ == 0)
						spa_pod_builder_raw(b, a1, size);
					spa_pod_builder_raw(b, a1, size);
				}
			}
		}
	}
	else if ((p1c == SPA_CHOICE_None && p2c == SPA_CHOICE_Range) ||
	    (p1c == SPA_CHOICE_Enum && p2c == SPA_CHOICE_Range) ||
	    (p1c == SPA_CHOICE_None && p2c == SPA_CHOICE_Step) ||
	    (p1c == SPA_CHOICE_Enum && p2c == SPA_CHOICE_Step)) {
		void *min = SPA_PTROFF(alt2,size,void);
		void *max = SPA_PTROFF(min,size,void);
		void *step = p2c == SPA_CHOICE_Step ? SPA_PTROFF(max,size,void) : NULL;
		bool found_def = false;

		/* we should prefer the alt2 range default value but only if valid */
		for (j = 0, a1 = alt1; j < nalt1; j++, a1 = SPA_PTROFF(a1,size,void)) {
			if (spa_pod_compare_value(type, alt2, min, size) < 0 ||
			    spa_pod_compare_value(type, alt2, max, size) > 0)
				break;
			if (spa_pod_compare_value(type, a1, alt2, size) == 0) {
				/* it is in the enum, use as default then */
				spa_pod_builder_raw(b, a1, size);
				found_def = true;
				break;
			}
		}
		/* copy all values inside the range */
		for (j = 0, a1 = alt1; j < nalt1; j++, a1 = SPA_PTROFF(a1,size,void)) {
			if ((res = spa_pod_filter_is_in_range(type, a1, min, max, step, size)) < 0)
				return res;
			if (res == 0)
				continue;
			if (n_copied++ == 0 && !found_def)
				spa_pod_builder_raw(b, a1, size);
			spa_pod_builder_raw(b, a1, size);
		}
	}
	else if ((p1c == SPA_CHOICE_Range && p2c == SPA_CHOICE_None) ||
	    (p1c == SPA_CHOICE_Range && p2c == SPA_CHOICE_Enum) ||
	    (p1c == SPA_CHOICE_Step && p2c == SPA_CHOICE_None) ||
	    (p1c == SPA_CHOICE_Step && p2c == SPA_CHOICE_Enum)) {
		void *min = SPA_PTROFF(alt1,size,void);
		void *max = SPA_PTROFF(min,size,void);
		void *step = p1c == SPA_CHOICE_Step ? SPA_PTROFF(max,size,void) : NULL;

		/* copy all values inside the range, this will automatically prefer
		 * a valid alt2 value */
		for (j = 0, a2 = alt2; j < nalt2; j++, a2 = SPA_PTROFF(a2,size,void)) {
			if ((res = spa_pod_filter_is_in_range(type, a2, min, max, step, size)) < 0)
				return res;
			if (res == 0)
				continue;
			if (n_copied++ == 0)
				spa_pod_builder_raw(b, a2, size);
			spa_pod_builder_raw(b, a2, size);
		}
	}
	else if ((p1c == SPA_CHOICE_Range && p2c == SPA_CHOICE_Range) ||
	    (p1c == SPA_CHOICE_Range && p2c == SPA_CHOICE_Step) ||
	    (p1c == SPA_CHOICE_Step && p2c == SPA_CHOICE_Range) ||
	    (p1c == SPA_CHOICE_Step && p2c == SPA_CHOICE_Step)) {
		void *min1 = SPA_PTROFF(alt1,size,void);
		void *max1 = SPA_PTROFF(min1,size,void);
		void *min2 = SPA_PTROFF(alt2,size,void);
		void *max2 = SPA_PTROFF(min2,size,void);

		/* max of min */
		if (spa_pod_compare_value(type, min1, min2, size) < 0)
			min1 = min2;
		/* min of max */
		if (spa_pod_compare_value(type, max2, max1, size) < 0)
			max1 = max2;

		/* reject impossible range */
		if (spa_pod_compare_value(type, max1, min1, size) < 0)
			return -EINVAL;

		/* prefer alt2 if in new range */
		a1 = alt2;
		if ((res = spa_pod_filter_is_in_range(type, a1, min1, max1, NULL, size)) < 0)
			return res;
		if (res == 0) {
			/* try alt1 otherwise */
			a1 = alt1;
			if ((res = spa_pod_filter_is_in_range(type, a1, min1, max1, NULL, size)) < 0)
				return res;
			/* fall back to new min value then */
			if (res == 0)
				a1 = min1;
		}

		spa_pod_builder_raw(b, a1, size);
		spa_pod_builder_raw(b, min1, size);
		spa_pod_builder_raw(b, max1, size);
		nc->body.type = SPA_CHOICE_Range;
	}
	else if ((p1c == SPA_CHOICE_None && p2c == SPA_CHOICE_Flags) ||
	    (p1c == SPA_CHOICE_Flags && p2c == SPA_CHOICE_None) ||
	    (p1c == SPA_CHOICE_Flags && p2c == SPA_CHOICE_Flags)) {
		if (spa_pod_filter_flags_value(b, type, alt1, alt2, size) != 1)
			return -EINVAL;
		nc->body.type = SPA_CHOICE_Flags;
	}
	else if (p1c == SPA_CHOICE_Range && p2c == SPA_CHOICE_Flags)
		return -ENOTSUP;
	else if (p1c == SPA_CHOICE_Enum && p2c == SPA_CHOICE_Flags)
		return -ENOTSUP;
	else if (p1c == SPA_CHOICE_Step && p2c == SPA_CHOICE_Flags)
		return -ENOTSUP;
	else if (p1c == SPA_CHOICE_Flags && p2c == SPA_CHOICE_Range)
		return -ENOTSUP;
	else if (p1c == SPA_CHOICE_Flags && p2c == SPA_CHOICE_Step)
		return -ENOTSUP;
	else if (p1c == SPA_CHOICE_Flags && p2c == SPA_CHOICE_Enum)
		return -ENOTSUP;

	if (nc->body.type == SPA_CHOICE_None) {
		if (n_copied == 0) {
			return -EINVAL;
		} else if (n_copied == 1) {
			/* we always copy the default value twice, so remove it
			 * again when it was the only one added */
			spa_pod_builder_remove(b, size);
		} else if (n_copied > 1) {
			nc->body.type = SPA_CHOICE_Enum;
		}
	}
	spa_pod_builder_pop(b, &f);

	return 0;
}

SPA_API_POD_FILTER int spa_pod_filter_part(struct spa_pod_builder *b,
	       const struct spa_pod *pod, uint32_t pod_size,
	       const struct spa_pod *filter, uint32_t filter_size)
{
	const struct spa_pod *pp, *pf;
	int res = 0;

	pf = filter;

	SPA_POD_FOREACH(pod, pod_size, pp) {
		bool do_copy = false, do_advance = false;
		uint32_t filter_offset = 0;
		struct spa_pod_frame f;

		switch (SPA_POD_TYPE(pp)) {
		case SPA_TYPE_Object:
			if (pf != NULL) {
				struct spa_pod_object *op = (struct spa_pod_object *) pp;
				struct spa_pod_object *of = (struct spa_pod_object *) pf;
				const struct spa_pod_prop *p1, *p2;

				if (SPA_POD_TYPE(pf) != SPA_POD_TYPE(pp))
					return -EINVAL;

				spa_pod_builder_push_object(b, &f, op->body.type, op->body.id);
				p2 = NULL;
				SPA_POD_OBJECT_FOREACH(op, p1) {
					p2 = spa_pod_object_find_prop(of, p2, p1->key);
					if (p2 != NULL)
						res = spa_pod_filter_prop(b, p1, p2);
					else if ((p1->flags & SPA_POD_PROP_FLAG_MANDATORY) != 0)
						res = -EINVAL;
					else
						spa_pod_builder_raw_padded(b, p1, SPA_POD_PROP_SIZE(p1));
					if (res < 0)
						break;
				}
				if (res >= 0) {
					p1 = NULL;
					SPA_POD_OBJECT_FOREACH(of, p2) {
						p1 = spa_pod_object_find_prop(op, p1, p2->key);
						if (p1 != NULL)
							continue;
						if ((p2->flags & SPA_POD_PROP_FLAG_MANDATORY) != 0)
							res = -EINVAL;
						if (res < 0)
							break;
						spa_pod_builder_raw_padded(b, p2, SPA_POD_PROP_SIZE(p2));
					}
				}
				spa_pod_builder_pop(b, &f);
				do_advance = true;
			}
			else
				do_copy = true;
			break;

		case SPA_TYPE_Struct:
			if (pf != NULL) {
				if (SPA_POD_TYPE(pf) != SPA_POD_TYPE(pp))
					return -EINVAL;

				filter_offset = sizeof(struct spa_pod_struct);
				spa_pod_builder_push_struct(b, &f);
				res = spa_pod_filter_part(b,
					SPA_PTROFF(pp,filter_offset,const struct spa_pod),
					SPA_POD_SIZE(pp) - filter_offset,
					SPA_PTROFF(pf,filter_offset,const struct spa_pod),
					SPA_POD_SIZE(pf) - filter_offset);
			        spa_pod_builder_pop(b, &f);
				do_advance = true;
			}
			else
				do_copy = true;
			break;

		default:
			if (pf != NULL) {
				if (SPA_POD_SIZE(pp) != SPA_POD_SIZE(pf))
					return -EINVAL;
				if (memcmp(pp, pf, SPA_POD_SIZE(pp)) != 0)
					return -EINVAL;
				do_advance = true;
			}
			do_copy = true;
			break;
		}
		if (do_copy)
			spa_pod_builder_raw_padded(b, pp, SPA_POD_SIZE(pp));
		if (do_advance) {
			pf = (const struct spa_pod*)spa_pod_next(pf);
			if (!spa_pod_is_inside(filter, filter_size, pf))
				pf = NULL;
		}
		if (res < 0)
			break;
	}
	return res;
}

SPA_API_POD_FILTER int
spa_pod_filter(struct spa_pod_builder *b,
	       struct spa_pod **result,
	       const struct spa_pod *pod,
	       const struct spa_pod *filter)
{
	int res;
	struct spa_pod_builder_state state;

        spa_return_val_if_fail(pod != NULL, -EINVAL);
        spa_return_val_if_fail(b != NULL, -EINVAL);

	spa_pod_builder_get_state(b, &state);
	if (filter == NULL)
		res = spa_pod_builder_raw_padded(b, pod, SPA_POD_SIZE(pod));
	else
		res = spa_pod_filter_part(b, pod, SPA_POD_SIZE(pod), filter, SPA_POD_SIZE(filter));

	if (res < 0) {
		spa_pod_builder_reset(b, &state);
	} else if (result) {
		*result = (struct spa_pod*)spa_pod_builder_deref(b, state.offset);
		if (*result == NULL)
			res = -ENOSPC;
	}
	return res;
}

/**
 * \}
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SPA_POD_FILTER_H */

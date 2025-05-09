# Tests

The pages shows the current status of what has been tested with respect to the
MILAN, AVB, and IEEE specifications.

## Hardware used

### Setup 1

* AMD Ryzen 7 7800x3d virtualized 8 cores
* 8GiB RAM
* i210 Passthrough VM

### Setup 2

* Intel(R) Xeon(R) CPU E3-1225 v5 @ 3.30GHz
* 8GiB RAM
* i210

## Relevant documents

* [Milan Specification](https://avnu.org/resource/milan-specification/)
* [IEEE 802.1Q-2014](https://ieeexplore.ieee.org/document/6991462)
* [IEEE 1722.1-2021](https://ieeexplore.ieee.org/document/9744630)
* [IEEE 1722-2016](https://ieeexplore.ieee.org/document/7782716)
* [IEEE 802.1AS-2011](https://ieeexplore.ieee.org/document/5741898)
* [IEEE 802.1BA-2011](https://ieeexplore.ieee.org/document/6032690)

## Methodology

The *specification related* tests are defined into their own
[Gherkin test plan](../../../../test-milan/). More tests need to be done and
added in order to validate the full interoperability of the solution.

### Specification related tests

| Test | Result | Note | Test Plan |
|-|-|-|-|
| Get Entity Name | ❌ FAIL | Entity responds with characters that are not UTF-8 | get_name.feature |
| Get Entity Name from different Entity | ✅ PASS | No response within 250ms | get_name.feature |
| Get STREAM_INPUT Counters | ✅ PASS  | N/A | get_counters.feature |
| Get STREAM_OUTPUT Counters | ✅ PASS  | N/A | get_counters.feature |
| Get AVB_INTERFACE Counters | ✅ PASS  | N/A | get_counters.feature |
| Get CLOCK_DOMAIN Counters | ✅ PASS  | N/A | get_counters.feature |
| Get CLOCK_DOMAIN Counters from ADP and ACMP multicast address | ❌ FAIL | Device responds to multicast address traffic | get_counters.feature |

## Audio tests

In the current settings, no profesional tools were used to verify and test the
audio quality that PW has produced.

The test will be splitted into, two categories: Listneer and talker.

### Listener

The listener has a very good quality of Audio. As for now, no **AUDIBLE** glitch has
been heard so far.

| Test | Result | Note |
|-|-|-|
| Audio sourced from an AVB-certified device | ✅ PASS | N/A |
| Audio Quality with no audible glitches | ✅ PASS | N/A |

### Talker

The Talker, on the other end has a very poor quality of Audio for now,
many **AUDIBLE** glitch has been heard so far, but audio is still going through.

| Test | Result | Note |
|-|-|-|
| Audio sinked to an AVB-ceritifed device |  ✅ PASS | N/A |
| Audio Quality with no audible glitches |  ❌ FAIL | N/A |

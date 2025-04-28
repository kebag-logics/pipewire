Feature: Returns counters for a specific descriptor

Background:
    # IEEE 1722.1-2021, Sec. 9.2.2. Common PDU
    Given AECP commands shall be sent unicast to the entity's MAC address
    # IEEE 1722.1-2021, Sec. 9.3.2.6. ATDECC Command Timeouts
    And a response to an AECP command shall be received within 250 milliseconds

Scenario: Get STREAM_INPUT Counters
Given the entity with descriptor type "STREAM_INPUT" and descriptor index "0" is online and running
When the get_counters command is sent with a correctly populated frame (descriptor_type=STREAM_INPUT, descriptor_index=0)
Then the response shall match the expected mandatory counter bitmap
# Milan v1.2, Table 5.16: GET_COUNTERS Stream Input counters.
# Expected
# Bit# Bit value Symbol
# 31 0x00000001 MEDIA_LOCKED
# 30 0x00000002 MEDIA_UNLOCKED
# 29 0x00000004 STREAM_INTERRUPTED
# 28 0x00000008 SEQ_NUM_MISMATCH
# 27 0x00000010 MEDIA_RESET
# 26 0x00000020 TIMESTAMP_UNCERTAIN
# 23 0x00000100 UNSUPPORTED_FORMAT 
# 22 0x00000200 LATE_TIMESTAMP 
# 21 0x00000400 EARLY_TIMESTAMP 
# 20 0x00000800 FRAMES_RX 
# Expected: 0b0000 00000 00000 00000 0000 1111 0011 1111
# Sent: 91e0f001000000e04c68018322f0fb000018001b21fffededfd9b16b00b5000000000000002900050000000000000000000000000000000000000000

Scenario: Get STREAM_OUTPUT Counters
Given the entity with descriptor type "STREAM_OUTPUT" and descriptor index "0" is online and running
When the get_counters command is sent with a correctly populated frame (descriptor_type=STREAM_OUTPUT, descriptor_index=0)
Then the response shall match the expected mandatory counter bitmap
# Milan v1.2, Table 5.17: GET_COUNTERS Stream Output counters.
# Expected
# Bit# Bit value Symbol
# 31 0x00000001 STREAM_START 
# 30 0x00000002 STREAM_STOP 
# 29 0x00000004 MEDIA_RESET  
# 28 0x00000008 TIMESTAMP_UNCERTAIN 
# 27 0x00000010 FRAMES_TX 
# Expected: 0000 0000 0000 0000 0000 0000 0001 1111
# Sent: 001b21dedfd900e04c68018322f0fb000018001b21fffededfd9b16b00b5000000000000002900060000000000000000000000000000000000000000

Scenario: Get AVB_INTERFACE Counters
Given the entity with descriptor type "AVB_INTERFACE" and descriptor index "0" is online and running
When the get_counters command is sent with a correctly populated frame (descriptor_type=AVB_INTERFACE, descriptor_index=0)
Then the response shall match the expected mandatory and optional counter bitmap
# Milan v1.2, Table 5.13: GET_COUNTERSmandatory AVB Interface counters.
# Expected
# Bit# Bit value Symbol
# 31 0x00000001 LINK_UP
# 30 0x00000002 LINK_DOWN
## Optional
# 29 0x00000004 FRAMES_TX
# 28 0x00000008 FRAMES_RX
# d27 0x00000010 RX_CRC_ERROR
## Mandatory
# 26 0x00000020 GPTP_GM_CHANGED
# Expected with optional: 0000 0000 0000 0000 0000 0000 0011 1111
# Sent: 001b21dedfd900e04c68018322f0fb000018001b21fffededfd9b16b00b5000000000000002900090000000000000000000000000000000000000000

Scenario: Get CLOCK_DOMAIN Counters
Given the entity with descriptor type "CLOCK_DOMAIN" and descriptor index "0" is online and running
When the get_counters command is sent with a correctly populated frame (descriptor_type=CLOCK_DOMAIN, descriptor_index=0)
Then the response shall match the expected mandatory counter bitmap
# Milan v1.2, Table 5.15: GET_COUNTERS Clock Domain counters.
# Expected
# Bit Bit value Symbol
# 31 0x00000001 LOCKED
# 30 0x00000002 UNLOCKED
# Expected: 0000 0000 0000 0000 0000 0000 0000 0011
# Sent: 001b21dedfd900e04c68018322f0fb000018001b21fffededfd9b16b00b5000000000000002900240000000000000000000000000000000000000000

Scenario: Get CLOCK_DOMAIN Counters from ADP and ACMP multicast address
Given the entity with descriptor type "CLOCK_DOMAIN" and descriptor index "0" is online and running
When the get_counters command is sent with an incorrectly populated frame (dst_mac=91:e0:f0:01:00:00, descriptor_type=CLOCK_DOMAIN, descriptor_index=0)
Then no valid get_counters_response shall be received within 250 milliseconds

# TODO: negative testing (out of bounds), optional counters for AVB Interface
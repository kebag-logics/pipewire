Feature: Returns name for a specific descriptor

Background:
    # IEEE 1722.1-2021, Sec. 9.2.2. Common PDU
    Given AECP commands shall be sent unicast to the entity's MAC address
    # IEEE 1722.1-2021, Sec. 9.3.2.6. ATDECC Command Timeouts
    And a response to an AECP command shall be received within 250 milliseconds

Scenario: Get Entity Name
Given the entity with descriptor type "ENTITY" and descriptor index "0" is online and running
When the get_name command is sent with a correctly populated frame (descriptor_type=ENTITY, descriptor_index=0, name_index=0)
Then the get_name_response is received within 200ms
    And the get_name response with thee correctly populated fields is received
    And the received name is matching the entity name
# Sent frame: 001b21dedfd900e04c68018322f0fb000018001b21fffededfd9b16b00b5000000000000001100000000000000000000000000000000000000000000

Scenario: Get Entity Name from different Entity
Given the entity with descriptor type "ENTITY" and descriptor index "0" is online and running
When the get_name command is sent with a correctly populated frame (descriptor_type=ENTITY2, descriptor_index=0, name_index=0)
Then no response is expected

# TODO: Add tests for locked entity and negative tests: out of bounds, ADP multicast address
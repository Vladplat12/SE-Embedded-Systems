Program is testing the peripherals on the STM32F746ZG card in order to insure hardware correctness.
We take part in card’s hardware verification for
the following peripherals:
* UART
* I2C
* ADC

The verification resembles a client server program, the testing program
(server) running on the P.C, sends commands using a propriety protocol
over UDP (IP/Ethernet) to the UUT.

Once the UUT Testing program receives the proprietary protocol, finds
out the peripheral to be tested and starts running the test.

Testing Procedure:
  1. The Testing program running on the P.C. (the Server) communicates
with UUT (the client) through the UDP/IP communication protocol.
  2. The server (Hercules) will send the commands (Perform the unit testing) on the
UUT (the Client), and wait for the response from the UUT.
  3. The UUT Ethernet device will receive the command and using the
LWIP stack pass it to the "UUT Testing Program"
  4. The test at the UUT is done by acquiring the needed parameters from
the incoming command, such as:
   * Which peripheral is about to be tested,
   * What is the string of characters sent to the peripheral.
   * Test Iteration (number of times test has to be run at the UUT).
  5. Once the UUT Testing has all the data needed for the test it will run
the test for the number of defined iterations, each test will result in
success or failure, each peripheral will have its own set of tests.
  6. The UUT test result per peripheral will be sent to the P.C. Testing
program.

Proprietary Protocols:
1. The command sent from the P.C. Testing Program to the UUT will
contain the following:
  * Test-ID – 4 Bytes.
  * Peripheral to be tested – 1 Byte (a bitfield for the peripheral
being tested: 2 – UART, I2C – 8, ADC – 16).
  * Iterations – 1 Byte (the number of iterations the test should run
at the UUT).
  * Bit pattern length – 1 Byte (the size of the Bit pattern string
sent to UUT).
  * Bit pattern – Bit pattern length (the actual string of characters
sent to the UUT).
2. The result protocol sent from the UUT back to the P.C. Testing
Program will contain the following:
  * Test-ID – 4 Bytes (a number given to the test so it'll be easy to
map it to the later on test result).
  * Test Result – test succeeded, test failed, error in sent packet.


UUT Testing Program (Client Side):
1. Once the UUT Testing Program receives the test command, it will
acquire the needed parameters and initiate the test on the required
peripheral.
2. Tests will vary between peripherals; the following includes Unit Testing for every Peripheral:

UART, I2C:
* The below procedure is described for UART testing but it
stands to I2C.
* Peripheral testing is required to be done using DMA
mode if possible.
* Each Peripheral testing will require peripheral
parameters (i.e. for UART you can assume BAUD
Rate 115200, 8bit Data, 1 Stop Bit, No parity).
* For the amount of needed iteration, the Testing Program
will send the received Bit Pattern to the UART0, which
in turn will pass the data to the UART1 port on the UUT.
UART1 will send back the received string to UART0
(predefined program waiting for incoming data).
* For every iteration, the UUT Testing program will
receive the incoming data from UART0 and compare it to
sent data.
* It the testing has been successful for all iterations; a
success result should be sent to the Testing P.C. Program.
* If at any time during the iterations a test has failed,
testing should be stopped and a Failure result should be
sent to the P.C. Testing Program.
* Calculate how long does it take to preform each I2C
transmit and receive with 0.1 second and in 0.5 second
resolution (separately).

ADC:
    * ADC required parameters 12 bit ADC.
    * Running the test beforehand, we should already have the
bitstream for the analog to digital conversion at the
current voltage.
    * For each iteration Run the conversion and compare the
ADC result with already known result.
    * Send the P.C. Testing Program the final test result (after
all the iterations).




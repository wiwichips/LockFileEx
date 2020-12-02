# Testing Framework


Please quit both shells after each test. Please use the same file when running each shell.


1. Test Objective: Ensure the program compiles.

Test: enter the make command into the console on mac or linux

Expected output: the program will compile without warnings or compilation errors.

Determine if the code passed: if it follows the expected output.

Succeed: My code succeeds at this test.

   2. Test Objective: test if an exclusive lock can be set and prevents another process from locking the range of bytes.

Test: Open two shells. In shell 1, set a shared lock with an offset of 10 for a length of 1 byte. In shell 2, set a shared lock. 

Shell 1:        X 10 1
Shell 2:        X 10 1

Expected output: The first shell should set a lock successfully. The second shell will hang because the first shell has set a lock.

Determine if the code passed: The test passes if the expected output occurs. 

Succeed: My code succeeds at this test.

      3. Test Objective: ensure there are no memory leaks.

Test: Run the program in valgrind. Set an exclusive lock with an offset of 10 and a length of 20 and unlock it. Test for a lock with an offset of 10 and a length of 20. Quit the program.


X 10 20
U 10 20
T 10 20
q

Expected output: 0 memory leaks.

Determine if the code passed: If the expected output is what the program outputs. 

Succeed: My code succeeds at this test. 

         4. Test Objective: Check that multiple locks can be set on the file from two different shells that don’t overlap. 

Test: Open two shells. 

Shell 1                X 0 1
Shell 2                X 1 1

Expected output: Both locks will be set as the locks are not overlapping. 

Determine if the code passed: if the expected output works

Succeed: My code succeeds at this test. 


            5. Test Objective: check to see if the test functionality can return information about an existing shared lock.

Test: open two shells and enter the following type, offset and length.


Shell 1                S 40 3
Shell 2                T 0 100

Expected output: shell 1 will successfully set a lock, shell 2 will output information about the shared lock such as that it is a shared lock, and that it has an offset of 40 and a length of 3 bytes.

Determine if the code passed: if the expected output occurs.

Succeed: My code succeeds at this test. 

               6. Test Objective: check to see if the test functionality can return information about an existing exclusive lock.

Test: open two shells and enter the following type, offset and length.


Shell 1                X 40 3
Shell 2                T 0 100

Expected output: shell 1 will successfully set a lock, shell 2 will output information about the exclusive lock such as that it is an exclusive lock, and that it has an offset of 40 and a length of 3 bytes.

Determine if the code passed: if the expected output occurs.

Succeed: My code succeeds at this test. 


                  7. Test Objective: check to see if a block is available after being unlocked.

Test: open two shells.

Shell 1                X 20 1
Shell 2                T 20 1
Shell 1                U 20 1
Shell 2                T 20 1

Expected output: the expected output will be for the lock to successfully be placed on shell 1, then for shell two to say that there is indeed a lock placed, then for shell 1 to successfully unlock, and for shell 2 to say there is no lock.

Determine if the code passed: if it followed the expected output it passed.

Succeed: My code succeeds at this test. 



                     8. Test objective: Check if every combination of exclusive and shared lock results in the correct behaviour. 

Test:

For each of these scenarios, test the following: on shell 1 set a lock with an offset of 0 and a length of 1, on shell 2 test the byte with an offset of 0 and a length of 1, on shell 2 attempt to set a lock with an offset of 0 and a length of 1, on shell 1 unlock an offset of 0 with a length of 1, on shell 2 test for a lock with an offset of 0 and a length of 1, then quit on both shells. 

                        1. Shared lock set, exclusive lock tested on another shell
                        2. Shared lock set, shared lock tested on another shell
                        3. Exclusive lock set, shared lock tested on another shell
                        4. Exclusive lock set, exclusive lock tested on another shell

Expected output: For shell two, when testing the lock it will output information about the lock that is valid. When trying to set the lock after the lock has been set by shell 1, it will hang if both shells are trying to set an exclusive lock. When the lock has been unlocked on shell one, shell two will then set its lock if it was hanging. Then when testing the lock it will say there is no lock.

Determine if the code passed: 

Succeed: My code succeeds at this test.

                        9.
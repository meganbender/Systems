# Systems
#### Course assignments for Systems Programming

### Assign1
  - WARNING: this has a fork bomb do NOT run
  - Goal: Create the program that spawns processes(Assign1).  Your program should try to spawn processes based on the command line
  arguments passed. Only allow for 10 command line arguements. Should allow the user to terminate child processes or quit and terminat
  all.
  - Issues: during the fork bomb had a user pass the name which would be inccorect since visual studio automatically puts arguement 1 when
    running from the inside the ide

### Donut Shop
  - Goal: Write a program that simulates a donut shop, there will be 2 bakers, 4 line managers, and 6 workers. Using Mutexes,
    Counting Semaphores, and Events to properly protect shared resources.

### File Insertion Sort
  - Goal: Using the command line (*programName #[for child processes] <unsorted.txt >sorted.txt*) take a file from standard in, sort
    the data, and dump to standard out. The data will be in the form of records 64 bytes in length, where the first 8 bytes are the key
    (i.e., sort by this key). If there are no command line options for how many child processes will be there use the # of cores (but take
    the next power of 2) on the computer.

### Memory Maps
  - Goal: Using Memory Maps to sort the same data from the file insertion program, the command line will be the same.
  - Issues:
      - line 380 should be "quickSort_func(low, high, pDestFile); //high-1"
      - lines 123 & 124 should be "int curList1 = low; int endList1 = mid-1;  int curList2 = mid; int endList2 = high;"
   
### Producer Consumer
  - Goal: Encrypting and decrypting files. Implementing a producer-consumer model by protecting shared resources using Counting and Binary
    Semaphores. 

### Run Transactions
  - xxxxx

### Find
  - Goal: Finding patterns using threads. Start a thread for each pattern to e found from the command line (e.x. command line: find kig
    kastisgreat king). Parent thread will start each of the threads and then wait for the child threads to finish.

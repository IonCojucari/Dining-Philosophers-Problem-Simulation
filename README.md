# Project: Dining Philosophers Problem Simulation

This project simulates the classic dining philosophers problem using threads and sockets, with a network-based client-server interaction. The objective is to manage philosopher clients accessing shared resources (forks) to eat, while preventing conflicts. This solution incorporates concurrency mechanisms and collects detailed statistics.

## Structure and Features

### Main Files
- **cli_automatique.c**: This file contains the client code, simulating the behavior of a philosopher. The client sends commands to the server to request and release forks, observing wait times and managing shared resources.
- **simu.c**: This file implements the server, which handles philosopher requests for forks, ensures synchronization via mutexes, and collects statistics for each client.

### Server Features
The server accepts connections from clients and manages fork availability for each connected philosopher. It records and calculates statistics for each philosopher, including:
- Maximum and average waiting time
- Total eating time
- Total number of meals
- Total simulation duration

### Client Features
Each client (philosopher) can perform the following actions:
1. **Request Forks**: The client sends a request to the server to obtain the necessary forks and waits for availability before proceeding to eat.
2. **Eat**: Once the forks are available, the client simulates eating for a random period while holding the forks.
3. **Release Forks**: After eating, the client releases the forks for others to use, then can repeat the process or exit the simulation.

### Output Files and Statistics
The program generates several output files:
- **PhilosopheXTempsReel.txt**: Real-time log of each philosopher's actions (requesting, obtaining, and releasing forks) with timestamps.
- **PhilosopheX.txt**: Summary statistics for each philosopher, such as average waiting time, total eating time, and number of meals.
- **DînerTest.txt**: Log of each philosopher's fork status, allowing verification of synchronization and access order.

### Example Statistics
Here’s an example of the statistics generated for one philosopher:
```
Maximum waiting time: 2.085938 seconds
Total time spent eating: 29.554688 seconds
Number of meals: 5
Average waiting time: 0.891406 seconds
Total time: 44.020864 seconds
```

## Compilation and Execution
A makefile is provided to simplify the project’s compilation and execution:
1. **Compilation**: Run `make` to compile the files.
2. **Execution**: Run `make run` to start the server and clients automatically. In some cases, you may need to use `sudo make run` to establish the connection.

## Purpose and Scope
This project demonstrates an application of concurrency and synchronization in a networked environment. It showcases skills in system programming, shared resource management, and performance analysis for multithreaded and networked applications.

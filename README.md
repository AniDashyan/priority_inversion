# Mars Pathfinder Priority Inversion Simulation

## Overview

This project simulates the **Mars Pathfinder priority inversion problem** in C++. It replicates how real-time systems can fail when thread scheduling is not handled properly and how **priority inheritance** solves the issue. The program runs two scenarios—**with** and **without** priority inheritance—and highlights the timing differences, showing how a low-priority task can block a high-priority one due to interference from a medium-priority thread.

## History: What Actually Happened on Mars?

In 1997, NASA's Mars Pathfinder was successfully deployed on Mars. But shortly after, its embedded system began **randomly resetting**, threatening the entire mission. The issue stemmed from a classic concurrency bug known as **priority inversion**.

### Mike Jones — “What Really Happened on Mars?”

In his now-famous article titled [*“What Really Happened on Mars?”*](https://web.archive.org/web/20120117113113/http://research.microsoft.com/en-us/um/people/mbj/Mars_Pathfinder/Mars_Pathfinder.html), Mike Jones (Director of the Microsoft PBG Team) provided a summary of the events that led to the discovery of the bug:

> A low-priority task was holding a shared resource. A high-priority task needed it, but got blocked. Meanwhile, a medium-priority task (unrelated to the shared resource) kept preempting the low-priority one, preventing it from releasing the resource.

This classic **priority inversion** led to resetting the system when the high-priority task didn’t complete in time.

### Glenn E. Reeves — Software Lead of Pathfinder

In response to public interest, [Glenn E. Reeves](https://users.cs.duke.edu/~carla/mars.html), the **actual Software Team Lead** for the Pathfinder mission, shared technical details:

* The high-priority thread was handling **critical spacecraft data**.
* The low-priority thread was managing **shared bus access**.
* The medium-priority thread was running frequent diagnostics and consuming CPU.

The bug had been identified **before launch**, but was considered rare. A **fix (priority inheritance)** had been implemented in the VxWorks RTOS used onboard, but **not activated** during initial testing. Once re-enabled remotely, the resets stopped, and the mission continued successfully.

This event remains one of the **most studied real-time concurrency bugs** in engineering education and space history.

##  Solution

This simulation replicates that scenario:

* 🟥 **High-priority thread**: Performs critical work and needs a shared mutex.
* 🟩 **Low-priority thread**: Holds the mutex for some time.
* 🟨 **Medium-priority thread**: Performs unrelated CPU-intensive work.

Without priority inheritance, the high-priority thread is blocked because:

* It waits on the mutex held by the low-priority thread.
* The low-priority thread can’t run because it keeps getting preempted by the medium-priority thread.

With **priority inheritance**, the low-priority thread **temporarily inherits the high priority**, finishes its work, and releases the resource quickly.

## Build & Run

Clone and build:

```bash
# Clone the repo
git clone https://github.com/AniDashyan/priority_inversion
cd priority_inversion

# Build the project
cmake -S . -B build
cmake --build build

# Run the executable
./build/main
```

##  Example Output
### The problem
```
Mars Pathfinder Priority Inversion Simulation
==============================================
Watch the HIGH thread wait times!


==================================================
PROBLEM (priority inversion problem)
==================================================
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
HIGH: Waited 0ms for resource
MEDIUM: Running background task...
LOW: Got resource
HIGH: Waited MEDIUM: Running background task...
0ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
LOW: Got resource
MEDIUM: Running background task...
HIGH: Waited 74ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
MEDIUM: Running background task...
LOW: Got resource
HIGH: Waited 96ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
MEDIUM: Running background task...
LOW: Got resource
MEDIUM: Running background task...
HIGH: Waited 65ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
LOW: Got resource
MEDIUM: Running background task...
HIGH: Waited 38ms for resource
MEDIUM: Running background task...

--------------------------------------------------
Press Enter to see the solution...
```

### The Solution
```
==================================================
SOLUTION (with priority inheritance)
==================================================
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
Warning: Failed to open thread handle on MinGW
LOW: Got resource (priority boosted!)
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
MEDIUM: Running background task...
LOW: Got resource (priority boosted!)
MEDIUM: Running background task...
HIGH: Waited 171ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
LOW: Got resourceMEDIUM: Running background task...
 (priority boosted!)
HIGH: Waited 62ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
MEDIUM: Running background task...
LOW: Got resource (priority boosted!)
MEDIUM: Running background task...
HIGH: Waited 79ms for resource
MEDIUM: Running background task...
HIGH: Waited 0ms for resource
LOW: Got resource (priority boosted!)
MEDIUM: Running background task...
HIGH: Waited 77ms for resource
MEDIUM: Running background task...

--------------------------------------------------

KEY OBSERVATION:
Without priority inheritance: HIGH thread waits longer
With priority inheritance: HIGH thread waits less
```

## How Does It Work?

The simulation uses a shared `std::mutex` to model contention:

* The **low-priority thread** locks the resource.
* The **high-priority thread** needs it for urgent computation.
* The **medium-priority thread** simulates CPU hogging, preempting the low-priority thread.

When **priority inheritance is disabled**, the high-priority thread suffers long delays—a direct result of the medium-priority thread interfering.

When **priority inheritance is enabled**, the system boosts the priority of the low-priority thread when it's holding a resource the high-priority thread is waiting for. This ensures timely completion and minimizes blocking.

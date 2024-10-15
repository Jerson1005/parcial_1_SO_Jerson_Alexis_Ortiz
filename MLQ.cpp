/*
 * Name: Jerson Alexis Ortiz Velasco                   
 * Course: Sistemas Operativos-01               
 * Version: 0.0.1                        
 * Date: 2024-10-15                    
 * Program Name: MLQ 
 */ 

#include <iostream>  // Includes the standard input-output library.
#include <fstream>   // Includes the file stream library to handle files.
#include <sstream>   // Includes the string stream library for string manipulation.
#include <vector>    // Includes the vector library to use dynamic arrays.
#include <queue>     // Includes the queue library to use FIFO structures.
#include <string>    // Includes the string library for string handling.
#include <algorithm> // Includes the algorithm library to use algorithms like sort.

using namespace std;

// Class representing a task with its attributes.
class Task
{
public:
    string id; // Identifier for the task.
    bool finished; // Indicates whether the task is finished.
    int burstT, arrivalT, q, priority, burstTInitial, waitT, completionT, responseTime, turnAroundT;

    // Constructor to initialize the task's attributes.
    Task(string i, int bt, int at, int q, int p)
        : id(i), burstT(bt), arrivalT(at), q(q), priority(p),
          burstTInitial(bt), waitT(0), completionT(0), responseTime(-1),
          turnAroundT(0), finished(false) {}
};

// Function to sort tasks by their arrival time.
bool sortByEntry(const Task &x, const Task &y)
{
    return x.arrivalT < y.arrivalT; // Returns true if x arrives before y.
}

// Structure to match tasks by their ID.
struct MatchById
{
    string id; // ID of the task to match.
    MatchById(const string &i) : id(i) {} // Constructor to initialize the ID.

    // Operator to check if the task ID matches.
    bool operator()(const Task &tsk) const
    {
        return tsk.id == id; // Checks if the task's ID matches.
    }
};

// Class that manages the execution and scheduling of tasks.
class Dispatcher
{
private:
    vector<Task> taskList; // List of tasks.
    queue<Task> roundQ1;   // Queue for Round Robin with quantum 3.
    queue<Task> roundQ2;   // Queue for Round Robin with quantum 5.
    vector<Task> fifoQ;    // List for First-Come-First-Serve tasks.
    int globalTime;        // Tracks the current global time.

public:
    Dispatcher() : globalTime(0) {} // Constructor that initializes the global time.

    // Function to add a task to the task list.
    void appendTask(Task t)
    {
        taskList.push_back(t); // Adds the task to the list.
    }

    // Function to update the details of a task in the task list.
    void updateTask(const Task &updatedTask)
    {
        for (vector<Task>::iterator it = taskList.begin(); it != taskList.end(); ++it)
        {
            if (it->id == updatedTask.id) // Checks if the ID matches.
            {
                *it = updatedTask; // Updates the task in the list.
                break;
            }
        }
    }

    // Function to run the task scheduling.
    void run()
    {
        // Distributes tasks into their respective queues.
        for (vector<Task>::iterator it = taskList.begin(); it != taskList.end(); ++it)
        {
            Task &t = *it; // Reference to the current task.
            switch (t.q) // Selects the queue based on the task type.
            {
            case 1:
                roundQ1.push(t); // Adds to roundQ1 if type 1.
                break;
            case 2:
                roundQ2.push(t); // Adds to roundQ2 if type 2.
                break;
            case 3:
                fifoQ.push_back(t); // Adds to fifoQ if type 3.
                break;
            }
        }

        // Executes Round Robin for each queue.
        executeRoundRobin(roundQ1, 3); // Executes Round Robin with quantum 3.
        executeRoundRobin(roundQ2, 5); // Executes Round Robin with quantum 5.
        executeFCFS(); // Executes First-Come-First-Serve.
    }

    // Function to execute the Round Robin scheduling.
    void executeRoundRobin(queue<Task> &rq, int quantum)
    {
        if (rq.empty()) // Returns if the queue is empty.
            return;

        queue<Task> activeQ = rq; // Creates a copy of the active queue.

        // Processes tasks in the Round Robin queue.
        while (!activeQ.empty())
        {
            Task t = activeQ.front(); // Gets the first task.
            activeQ.pop(); // Removes the task from the queue.

            // Checks if the task has arrived.
            if (t.arrivalT > globalTime)
            {
                globalTime = t.arrivalT; // Updates the global time if the task hasn't arrived yet.
            }

            // Sets the response time if it hasn't been set.
            if (t.responseTime == -1)
            {
                t.responseTime = globalTime; // Sets the response time.
            }

            // Calculates the time slice for this execution.
            int slice = (t.burstT < quantum) ? t.burstT : quantum; // Determines the slice size.
            t.burstT -= slice; // Reduces the burst time.
            globalTime += slice; // Updates the global time.

            // Checks if the task has finished.
            if (t.burstT == 0)
            {
                t.completionT = globalTime; // Sets the completion time.
                t.turnAroundT = t.completionT - t.arrivalT; // Calculates the turnaround time.
                t.waitT = t.turnAroundT - t.burstTInitial; // Calculates the waiting time.
                t.finished = true; // Marks the task as finished.

                updateTask(t); // Updates the task in the task list.
            }
            else
            {
                activeQ.push(t); // Re-adds the task to the queue for its next execution.
            }
        }
    }

    // Function to execute the First-Come-First-Serve scheduling.
    void executeFCFS()
    {
        if (fifoQ.empty()) // Returns if the queue is empty.
            return;

        sort(fifoQ.begin(), fifoQ.end(), sortByEntry); // Sorts tasks by arrival time.

        // Processes tasks in FIFO order.
        for (vector<Task>::iterator it = fifoQ.begin(); it != fifoQ.end(); ++it)
        {
            Task &t = *it; // Reference to the current task.

            // Checks if the task has arrived.
            if (t.arrivalT > globalTime)
            {
                globalTime = t.arrivalT; // Updates the global time if the task hasn't arrived yet.
            }

            // Sets the response time if it hasn't been set.
            if (t.responseTime == -1)
            {
                t.responseTime = globalTime; // Sets the response time.
            }

            t.waitT = globalTime - t.arrivalT; // Calculates the waiting time.
            globalTime += t.burstT; // Updates the global time with the burst time.
            t.completionT = globalTime; // Sets the completion time.
            t.turnAroundT = t.completionT - t.arrivalT; // Calculates the turnaround time.
            t.finished = true; // Marks the task as finished.

            updateTask(t); // Updates the task in the task list.
        }
    }

    // Function to export results to a file.
    void exportResults(const string &file)
    {
        ofstream resultFile(file); // Opens the file for writing.
        resultFile << "# id; BT; AT; Q; Pr; WT; CT; RT; TAT\n"; // Writes the headers.

        double totalWT = 0, totalCT = 0, totalRT = 0, totalTAT = 0; // Initializes totals.

        // Writes the results for each task.
        for (vector<Task>::iterator it = taskList.begin(); it != taskList.end(); ++it)
        {
            Task &t = *it; // Reference to the current task.
            resultFile << t.id << ";" << t.burstTInitial << ";" << t.arrivalT << ";" << t.q << ";" << t.priority << ";"
                        << t.waitT  << ";" << t.completionT << ";" << t.responseTime << ";" << t.turnAroundT  << "\n";

            // Sums the totals.
            totalWT += t.waitT;
            totalCT += t.completionT;
            totalRT += t.responseTime;
            totalTAT += t.turnAroundT;
        }

        size_t totalTasks = taskList.size(); // Gets the total number of tasks.

        // Writes the averages to the file.
        resultFile << "WT=" << totalWT / totalTasks << "; CT=" << totalCT / totalTasks << "; RT=" << totalRT / totalTasks << "; TAT=" << totalTAT / totalTasks << ";\n";
        resultFile.close(); // Closes the file.
        cout << "Results saved in " << file << endl; // Displays a message that results have been saved.
    }

    // Function to reset the dispatcher.
    void reset()
    {
        taskList.clear(); // Clears the task list.
        queue<Task> empty1, empty2;
        roundQ1.swap(empty1); // Clears the Round Robin 1 queue.
        roundQ2.swap(empty2); // Clears the Round Robin 2 queue.
        fifoQ.clear(); // Clears the FIFO queue.
        globalTime = 0; // Resets the global time.
    }
};

// Function to load tasks from a file.
void loadTasksFromFile(Dispatcher &dispatcher, const string &file)
{
    ifstream inputFile(file.c_str()); // Opens the file for reading.
    string record;

    // Reads the file line by line.
    while (getline(inputFile, record))
    {
        if (record.empty() || record[0] == '#') // Skips empty lines or comments.
            continue;

        stringstream taskStream(record); // Creates a string stream for the line.
        string id; // Task ID.
        int bt, at, q, pr; // Burst time, arrival time, queue, and priority.
        char delim;

        // Parses the record.
        getline(taskStream, id, ';'); // Gets the task ID.
        taskStream >> bt >> delim >> at >> delim >> q >> delim >> pr; // Gets the burst time, arrival time, queue, and priority.

        Task task(id, bt, at, q, pr); // Creates a new task.
        dispatcher.appendTask(task); // Adds the task to the dispatcher.
    }
}

// Main function to run the simulation.
int main()
{
    cout << "SIMULATION OF MLQ SCHEDULING ALGORITHM" << endl; // Displays the simulation message.
    cout << "Queue structure in ready:" << endl; // Displays the queue structure.
    cout << "Q1: RR(3)" << endl; // Round Robin 1 queue.
    cout << "Q2: RR(5)" << endl; // Round Robin 2 queue.
    cout << "Q3: FCFS" << endl; // First-Come-First-Serve queue.
    cout << "Executing programs: " << endl; // Displays the execution message.
    
    Dispatcher dispatcher; // Creates an instance of the dispatcher.
    dispatcher.reset(); // Resets the dispatcher.
    loadTasksFromFile(dispatcher, "mlq001.txt"); // Loads tasks from the file.
    dispatcher.run(); // Executes the scheduling.
    cout << "Program 1: mlq001.txt " << endl; // Displays the message for program 1.
    dispatcher.exportResults("mlq001OUT.txt"); // Exports the results to a file.

    // Repeats for the following files.
    dispatcher.reset();
    loadTasksFromFile(dispatcher, "mlq005.txt");
    dispatcher.run();
    cout << "Program 2: mlq005.txt " << endl;
    dispatcher.exportResults("mlq005OUT.txt");

    dispatcher.reset();
    loadTasksFromFile(dispatcher, "mlq019.txt");
    dispatcher.run();
    cout << "Program 3: mlq019.txt " << endl;
    dispatcher.exportResults("mlq019OUT.txt");
    return 0; // End of the program.
}



# Report of PERT project

Author: Hoyue
Blog: https://hoyue.fun
Blog Posts of the project: https://hoyue.fun/se_pert.html
The blog post will be automatically published after the deadline.

## Environment

The project based on Python 3.x, required environment:

* Python 3.x and upper version

* Python Packages at least need:

  * **os**: Provide file operations.
  * **json**: Provide JSON file operation.
  * **math**: Provide some math functions.
  * **scipy.stats**: Provides statistical functions and distributions.
  * **networkx**: Creating, manipulating, and analyzing complex networks graph.
  * **matplotlib.pyplot**: Provides functions for creating visualizations.
  * **queue**: Provides classes for implementing queues.
  * **tabulate**: Creating formatted tables from data, which can be useful for displaying data in a clear and organized way.

  Recommended to use **Anaconda**, an integrated environment contains many packages

* Data files follow **JSON format**



## Usage

Data file the default data file is **data.json** in the same directory as the code.

It is a list where every node is a dictionary, the example format of a dictionary is:

```json
{
	"Activity Code": "A",
	"Immediate Predecessor": ["B", "C"],
	"Optimistic Estimate": 12,
	"Most Likely Estimate": 15,
	"Pessimistic Estimate": 18
 }
```

Specifically, when there is no **Immediate Predecessor** for this activity, set to `[]` (empty list)

I provided two test data in the file folder: **data1.json(as same as data.json), data2.json**

![image-20230924160853476](https://image.hoyue.fun/imgup/2023/09/202309261942713.webp)

Open the **pert.py** in Terminal and it will show a menu.

You **must load data file** first. Press **1** or **2** to load file.

* Press 1: Load a default file **data.json**
* Press 2: Load a custom file which is located at the same folder of the program or an absolute path. e.g.![image-20230924162132570](https://image.hoyue.fun/imgup/2023/09/202309261948937.webp)

After you load a file successfully, you will see the notice: **Data loaded successfully!** and bring you back to menu.

If you input a wrong file, the program will report an error and exit.

Then, you can choose choice 3 - 9 to implement the corresponding function. e.g. (By default file data.json)

* Press 3: Calculate the estimated duration for each activity.

  ![image-20230924162938747](https://image.hoyue.fun/imgup/2023/09/202309261957575.webp)

* Press 4: Calculate the time for each activity including Earliest Start Time, Earliest Finish Time, Latest Start Time, Latest Finish Time, Slack Time and Whether it is in the critical path.

  ![image-20230924163127747](https://image.hoyue.fun/imgup/2023/09/202309261957966.webp)

* Press 5: Find the Critical Path.

  ![image-20230924163148052](https://image.hoyue.fun/imgup/2023/09/202309261958047.webp)

* Press 6: Calculate the expected project completion time.

  ![image-20230924163203136](https://image.hoyue.fun/imgup/2023/09/202309261958012.webp)

* Press 7: Calculate the probability of completing the project by a targeted time.

  Assume that we calculate the probability to complete the project in **larger** than **145** days.

  We input Targeted time is 145 and Distribution mode is l(larger).

  ![image-20230924163431159](https://image.hoyue.fun/imgup/2023/09/202309262002379.webp)

* Press 8: Calculate all the items(By default data).

  If you don't load a file, it will load default data and show before items. If you have loaded a file, it will calculate by your data.

* Press 9: Draw an activity graph.

  It will show in a new window.

  ![image-20230924163605263](https://image.hoyue.fun/imgup/2023/09/202309262003456.webp)

* Press 10: Exit.

  

## Design Idea

### Storage Design

I designed a list `PERT` to store input from a JSON file. Each of its items is a dictionary, initialized as `PERT = []`

Also define a class `Graph` whose constructor sets every object of class `Graph` to have a `graph` dictionary for information about this graph.

```python
# Define an array to store data and each item is a dictionary
PERT = []

class Graph:
    def __init__(self):
        self.graph = {}
```

Inside the function `file_load` is designed to read data from a JSON file and store it in the `PERT` array. The argument to the function `file_load` is a file directory, which can be passed in by user input, and the default value is `data.json`.
In the design, the path is determined first, if the path does not exist, an error message is returned, otherwise the data read is stored in the temporary variable `data`.

The next step is to iterate the entire `data` and read in the data one section at a time. The `Optimistic Estimate`, `Most Likely Estimate` and `Pessimistic Estimate` data can be used to directly calculate the `Estimate During` and the variance using the formulas. This information will be written as elements of the PERT table, segment by segment.

Finally, 1 is returned to indicate that the contents of the file were successfully read.

```python
# Load data from json file
def file_load(filename="data.json"):
    if os.path.exists(filename):
        with open(filename, "r") as data_table:
            data = json.load(data_table)
    else:
        print("Error, file not found!")
        print_separator()
        return 0

    # Iterate through data
    for item in data:
        activity_code = item['Activity Code']
        immidiate_predecessor = item['Immediate Predecessor']  # An array
        optimistic_estimate = item['Optimistic Estimate']
        most_likely_estimate = item['Most Likely Estimate']
        pessimistic_estimate = item['Pessimistic Estimate']
        estimate_during = (optimistic_estimate + 4 * most_likely_estimate + pessimistic_estimate) / 6
        variance = ((pessimistic_estimate - optimistic_estimate) / 6) ** 2

        PERT.append({"activity code": activity_code, "immidiate_predecessor": immidiate_predecessor, "estimate_during": estimate_during, "variance": variance})
    return 1
```

With the data read in, we still need to load the data into the graph, and I designed the member function `add_activity` within the Graph class. Its argument is the table `PERT` where we store the input.

When adding to the graph, in addition to adding some necessary information, I designed the `next_activity` list to store the backward nodes of the activity, making it easy to find `earliest start` and `earliest finish` later.

```python
# Add a node to the graph
    def add_activity(self, PERT):
        immidiate_predecessor = PERT['immidiate_predecessor']
        time = PERT['estimate_during']
        variance = PERT['variance']
        # Set predecessor to the new node. Next_activity is an array that stores the next activity.
        for i in immidiate_predecessor:
            if (i in self.graph):
                self.graph[i]["next_activity"].append(PERT['activity code'])
        # Add the new node to the graph
        self.graph[PERT['activity code']] = {
            "activity_code": PERT['activity code'],
            "estimate_during": time,
            "variance": variance,
            "next_activity": [],
            "before_activity": immidiate_predecessor,
            "earliest_start_time": 0,
            "earliest_finish_time": 0,
            "latest_start_time": 0,
            "latest_finish_time": 0,
            "slack_time": -1,
            "critical_path": False
        }
```



### Calculate the earliest start and earliest finish time

In the function `earliest_time`, we initialize the nodes with no predecessors as the starting point, and then compute `ES` and `EF` by postdating them.

In the first level iterates through the activity names in the object's `graph` dictionary one by one, and in the second level loops through the current activity's posterior, and if the posterior's `ES` is smaller than the current node's `EF`, the posterior's `ES` is replaced by the current node's `EF` and the posterior's `EF` is recalculated.

At the end of the loop the `ES` and `EF` of all activities are computed.

```python
# Calculate the earliest start time and earliest finish time
    def earliest_time(self):
        # Initialize the earliest start time and earliest finish time
        for i in self.graph:
            if (self.graph[i]["before_activity"] == []):
                self.graph[i]["earliest_finish_time"] = self.graph[i]["estimate_during"]
        # Caculate the earliest start time and earliest finish time
        for i in self.graph:
            for j in self.graph[i]["next_activity"]:
                if (self.graph[j]["earliest_start_time"] < self.graph[i]["earliest_finish_time"]):
                    self.graph[j]["earliest_start_time"] = self.graph[i]["earliest_finish_time"]
                self.graph[j]["earliest_finish_time"] = self.graph[j]["earliest_start_time"] + self.graph[j]["estimate_during"]
```



### Calculate the latest start and latest finish time

In the function `latest_time`, we similarly initialize the nodes with no descendants to the end first, and their `LF`s are all the largest `LF`s.

At the first level the activity names in the object's `graph` dictionary are traversed in reverse, and at the second level the precursor of the current activity is looped over, and if the precursor's `LF` is greater than the current node's `LS` the precursor's `LF` is replaced by the current node's `LS`, and the descendant's `LS` is recalculated.

The end of the loop then computes the `LS` and `LF` of all activities.

```python
# Calculate the latest start time and latest finish time
    def latest_time(self):
        # Initialize the latest start time and latest finish time
        estimate_time = 0
        for i in reversed(self.graph):
            if (self.graph[i]["next_activity"] == []):
                estimate_time = max(estimate_time, self.graph[i]["earliest_finish_time"])
                self.graph[i]["latest_finish_time"] = estimate_time
                self.graph[i]["latest_start_time"] = self.graph[i]["latest_finish_time"] - self.graph[i]["estimate_during"]

        # Calculate the latest start time and latest finish time
        for i in reversed(self.graph):
            for j in self.graph[i]["before_activity"]:
                if (self.graph[j]["latest_finish_time"] == 0 or self.graph[j]["latest_finish_time"] > self.graph[i]["latest_start_time"]):
                    self.graph[j]["latest_finish_time"] = self.graph[i]["latest_start_time"]
                self.graph[j]["latest_start_time"] = self.graph[j]["latest_finish_time"] - self.graph[j]["estimate_during"]
```



### Calculate the slack time

Calculating `slack time` is very simple, just calculate `FS - ES` for each item.

```python
# Calculate the slack time LS-ES
    def slack_time(self):
        # Calculate the slack time
        for i in self.graph:
            self.graph[i]["slack_time"] = self.graph[i]["latest_start_time"] - self.graph[i]["earliest_start_time"]

```



### Calculate the critical path

Determining whether an activity is a `critical path` or not is very simple; if the activity's `slack time` is 0, then it is a `critical path`, otherwise it is not.

```python
# Calculate the critical path that Slack = 0
    def critical_path(self):
        # Calculate the critical path
        for i in self.graph:
            if (self.graph[i]["slack_time"] == 0):
                self.graph[i]["critical_path"] = True
```



### Print

This section is a collection of outputs.

First of all, we should execute those computational functions before the output in order to have enough data for the output. Call those calculation functions in function `initization`.

```python
# Initialization before printing
    def initization(self):
        self.earliest_time()
        self.latest_time()
        self.slack_time()
        self.critical_path()
```



Then we output `Estimate duration`, which we output as a table, using `tabulate` to tabulate, write the data for each activity to `Estimate_table`, and set the header and table format.

```python
# Print the Estimate duration
    def print_estimate_duration(self):
        print("Estimate duration: ")
        Estimate_table = [["Activity", "Estimate Duration", "Variance"]]
        for i in self.graph:
            Estimate_table.append([self.graph[i]["activity_code"], self.graph[i]["estimate_during"], self.graph[i]["variance"]])
        print(tabulate(Estimate_table, headers="firstrow", tablefmt='fancy_grid', numalign="center"))
```



Similarly, outputting the entire table writes the necessary data to `table` and sets the header and table format.

```python
# Print the earliest/latest start time and finish time
    def print_PERTtable(self):
        print("PERT table: ")
        table = [["Activity", "Ealiest Start", "Ealiest Finish", "Latest Start", "Latest Finish", "Slack Time", "Critical Path"]]
        for i in self.graph:
            table.append([
                self.graph[i]["activity_code"],
                self.graph[i]["earliest_start_time"],
                self.graph[i]["earliest_finish_time"],
                self.graph[i]["latest_start_time"],
                self.graph[i]["latest_finish_time"],
                self.graph[i]["slack_time"],
                self.graph[i]["critical_path"]])
        print(tabulate(table, headers="firstrow", tablefmt='fancy_grid'))
```



Outputting `critical path` connects each `critical path` via `->`

```python
# Print the critical path
    def print_critical_path(self):
        print("Critical path: ")
        for i in self.graph:
            if (self.graph[i]["critical_path"]):
                print(self.graph[i]["activity_code"], end=" ")
                if (self.graph[i]["next_activity"] != []):
                    print("->", end=" ")
        print()
```



When outputting `expected project completion time`, you need to find the `LF` of the last active `LF` through the `keys()` property and output it.

```python
# Print the expected project completion time
    def print_expected_project_completion_time(self):
        keys = list(self.graph.keys())
        print("Expected project completion time: %s" % self.graph[keys[-1]]["latest_finish_time"])
```



### Calculate the Probability

To calculate the probability that the target completion time is **larger/smaller (l/s)** than the actual completion time, the function `probability` is designed. It has two parameters, one is the target completion time `targeted` and the other is the distribution, which can be **larger/smaller** **(l/s)**

Based on the formulas `Standard Deviation` and `Z-Score` can be calculated and the probability is calculated based on the `scipy.stats` module.

```python
# Calculate the probability of completing the project. mode = larger/smaller
    def probability(self, targeted, mode):
        if (mode == "l"):
            mode = "larger"
        elif (mode == "s"):
            mode = "smaller"
        var_sum = 0
        for i in self.graph:
            if (self.graph[i]["critical_path"]):
                var_sum += self.graph[i]["variance"]
        standard_deviation = math.sqrt(var_sum)
        keys = list(self.graph.keys())
        expected = self.graph[keys[-1]]["latest_finish_time"]
        zscore = (targeted - expected) / standard_deviation
        if (mode == "larger"):
            probability = 1 - st.norm.cdf(zscore)
        else:
            probability = st.norm.cdf(zscore)
        print("Z-score: {}\nProbability of project completion {} than {} is: {:.2%} ({}). ".format(zscore, mode, targeted, probability, probability))

```



### Draw the graph

In the function `graph_table` **G** is used as a grid graph `graph_dict` as the `graph` of the object, while a queue **Q** is designed for generating nodes and edges in FIFO. `pos` is used as a map to represent the coordinates of each node, and the `postion` dictionary records the `row` and `col` of each node in order to generate the coordinates of the nodes afterward.

First I first generate a horizontal line of multiple nodes, which represent the `critical path`, with initial node coordinates of (0,0), adding 1 in order, and adding edges, and attaching the labels of the edges (activity names). For each node added, this node goes into the queue so that its children can be generated sequentially thereafter.

Next set up an initialization node that points to the first node.

Nodes that are not `critical path` are then generated. A node generates a `range_col` based on the number of its descendants, which represents the maximum value of the vertical coordinates of its descendant nodes, with each descendant's vertical coordinate differing by 1. In the case where there is only one descendant, its descendant's vertical coordinate is the same as the current node.

Specifically, if the current node has no descendants and it is not the last `critical path`, then it points to the last `critical path`. (stating that it is the last bit and points directly to END)

Next iterates through all the descendants of the current node, and if the descendant is not in the list of nodes already added, then adds it, adds a solid edge and queues the descendant.

Otherwise, determine the size of the current node's `EF` and the descendant's `ES`, if the current node's `EF` is large, then you need to set the descendant's original edge to dashed and add a solid edge. If the `ES` of the descendant node is large, then add a dashed edge. 

Special judgment, if the current node's `EF` and the descendant node's `ES` are the same size, you need to set the descendant node's original **non-critical path's edges** as dashed lines and add a solid line edge.

Finally, the nodes, edges and labels of the edges are drawn by `matplotlib.pyplot` and `networkx` modules respectively, and finally shown.

```python
# Graph the table
    def graph_table(self):
        graph_dict = self.graph
        G = nx.DiGraph()
        pos = {}
        Q = queue.Queue()
        postion = {}

        # Set the critical path
        row = 0
        for i in graph_dict:
            if (graph_dict[i]["critical_path"]):
                Q.put(graph_dict[i]["activity_code"])
                G.add_node(graph_dict[i]["activity_code"])
                postion[graph_dict[i]["activity_code"]] = {"row": 0, "col": 0}
                pos[graph_dict[i]["activity_code"]] = (row, 0)
                row += 1
                postion[graph_dict[i]["activity_code"]]["row"] = row
                postion[graph_dict[i]["activity_code"]]["col"] = 0
                for j in graph_dict[i]["before_activity"]:
                    if(j in G.nodes()):
                        G.add_edge(graph_dict[j]["activity_code"], graph_dict[i]["activity_code"], name=graph_dict[i]["activity_code"])
        
        # Set the startpoint and endpoint
        G.add_node("Start")
        pos["Start"] = (-1, 0)
        G.add_edge("Start", Q.queue[0], name = Q.queue[0])
        last_critical_point = Q.queue[Q.qsize() - 1]
        
        dash = []
        # Set the non-critical path
        while not Q.empty():
            cur = Q.get()
            # print(cur)
            if len(graph_dict[cur]["next_activity"]) > 1:
                range_col = math.floor(math.log2(len(graph_dict[cur]["next_activity"])))
            elif len(graph_dict[cur]["next_activity"]) == 1:
                range_col = postion[cur]["col"]
            if graph_dict[cur]["next_activity"] == [] and cur != last_critical_point:
                G.add_edge(cur, last_critical_point)
                dash.append((cur, last_critical_point))
                continue
            
            for next_node in graph_dict[cur]["next_activity"]:
                if next_node not in G.nodes():
                    G.add_node(next_node)
                    col = range_col
                    row = (postion[cur]["row"]) * (1 - (col / 10.0)) + 1
                    postion[next_node] = {"row": row, "col": col}
                    pos[next_node] = (row, col)
                    range_col -= 1
                    if range_col == 0:
                        range_col -= 1
                    G.add_edge(cur, next_node, name=next_node)
                    Q.put(next_node)
                else:
                    cur_EF = graph_dict[cur]["earliest_finish_time"]
                    next_ES = graph_dict[next_node]["earliest_start_time"]
                    if cur_EF > next_ES:
                        pred = list(G.predecessors(next_node))
                        for j in range(len(pred)):
                            G.remove_edge(pred[j], next_node)
                            G.add_edge(pred[j], next_node)
                            dash.append((pred[j], next_node))
                        G.add_edge(cur, next_node, name=next_node)
                    elif cur_EF < next_ES:
                        for j in graph_dict[next_node]["before_activity"]:
                            if graph_dict[j]["critical_path"]:
                                G.add_edge(cur, j)
                                dash.append((cur, j))
                    else:
                        cur == next_node
                        pred = list(G.predecessors(next_node))
                        for j in range(len(pred)):
                            if graph_dict[pred[j]]["critical_path"] and not graph_dict[cur]["critical_path"]:
                                G.remove_edge(pred[j], next_node)
                                G.add_edge(pred[j], next_node)
                                dash.append((pred[j], next_node))
                                G.add_edge(cur, next_node, name=next_node)

        # Draw the graph
        nx.draw_networkx_nodes(G, pos)
        # nx.draw_networkx_edges(G, pos,label=True)
        dashed_edges = [(u,v) for u,v in G.edges if (u,v) in dash]
        nx.draw_networkx_edges(G, pos, edgelist=dashed_edges, style='dashed')
        solid_edges = [(u,v) for u,v in G.edges if (u,v) not in dash]
        nx.draw_networkx_edges(G, pos, edgelist=solid_edges, style='solid')
        nx.draw_networkx_edge_labels(G, pos, edge_labels=nx.get_edge_attributes(G,'name'),font_family="sans-serif", font_size=10)
        plt.show()
```



### Auxiliary function

There are some auxiliary functions in the program that assist in generating and determining some situations.

The `print_separator` function is used to generate separators to allow important output to be separated.

The `loading_test` function is used to test whether data has been loaded, and its parameter type is analyzed differently depending on the situation.
When Type = 0, the function will output the loading message, return 1 to indicate load successfully and output a success message, and 0 to indicate that it has already been loaded.
When Type = 2, the situation is very similar to Type = 0, the difference is that no action is taken when 1 is returned.
When Type = 1, it will check if it needs to be loaded first. Returning 1 means that it has already been loaded, returning 0 means that it needs to be loaded first, and outputs a message.

```python
# Print separator
def print_separator():
    print("\n------------------------------------------------------------\n")


# Loading test.
def loading_test(type):
    if type == 0:
        # Type = 0(load infomation -> return 1(load successfully infomation) or 0(has been loaded infomation) for default file
        if not PERT:
            print_separator()
            print("Data loaded successfully!")
            print_separator()
            return 1
        else:
            print_separator()
            print("Data has been loaded!")
            print_separator()
            return 0
    elif type == 1:
        # Type = 1(loading check -> return 1(print separator) or 0(need to load))
        if not PERT:
            print_separator()
            print("Please load data first!")
            print_separator()
            return 0
        else:
            print_separator()
            return 1
    elif type == 2:
        # Type = 2(for custom file like type 0)
        if not PERT:
            return 1
        else:
            print_separator()
            print("Data has been loaded!")
            print_separator()
            return 0

```





### Main function

The main function is relatively simple, mainly a menu with function calls and feedback based on user input.

```python
# Main function
# Initialization
PERT_graph = Graph()
print("Welcome to PERT project management system!\nAuthor: Hoyue")
while True:
    print("Menu: ")
    print("1. Load data from default file(data.json).")
    print("2. Load data from custom file.")
    print("3. Calculate the estimated duration for each activity.")
    print("4. Calculate the time for each activity including Earliest Start Time, Earliest Finish Time, Latest Start Time, Latest Finish Time, Slack Time and Whether it is in the critical path.")
    print("5. Find the Critical Path.")
    print("6. Calculate the expected project completion time.")
    print("7. Calculate the probability of completing the project by a targeted time.")
    print("8. Calculate all the items(By default data).")
    print("9. Draw an activity graph.")
    print("10. Exit.")
    choice = input("Please press the number to choose the function: ")

    if choice == "1":
        if loading_test(0):
            if file_load():
                for i in range(len(PERT)):
                    PERT_graph.add_activity(PERT[i])

    elif choice == "2":
        if loading_test(2):
            print_separator()
            filename = input("Please input your filename in the software folder(Format as data.json): ")
            if file_load(filename):
                for i in range(len(PERT)):
                    PERT_graph.add_activity(PERT[i])
                print("Data loaded successfully!")
                print_separator()
    elif choice == "3":
        if loading_test(1):
            PERT_graph.initization()
            PERT_graph.print_estimate_duration()
            print_separator()
    elif choice == "4":
        if loading_test(1):
            PERT_graph.initization()
            PERT_graph.print_PERTtable()
            print_separator()
    elif choice == "5":
        if loading_test(1):
            PERT_graph.initization()
            PERT_graph.print_critical_path()
            print_separator()
    elif choice == "6":
        if loading_test(1):
            PERT_graph.initization()
            PERT_graph.print_expected_project_completion_time()
            print_separator()
    elif choice == "7":
        if loading_test(1):
            PERT_graph.initization()
            print("For cauculating the probability of completing the project, please input the targeted time and distribution mode(larger/smaller).")
            targeted = float(input("Targeted time: "))
            mode = input("Distribution mode(larger/smaller or l/s): ")
            if (mode != "larger" and mode != "smaller" and mode != "l" and mode != "s"):
                print_separator()
                print("Error input! Please input again!")
                print_separator()
                continue
            PERT_graph.probability(targeted, mode)
            print_separator()
    elif choice == "8":
        if not PERT:
            file_load()
            for i in range(len(PERT)):
                PERT_graph.add_activity(PERT[i])
        print_separator()
        PERT_graph.initization()
        PERT_graph.print_estimate_duration()
        PERT_graph.print_PERTtable()
        PERT_graph.print_critical_path()
        PERT_graph.print_expected_project_completion_time()
        print("For cauculating the probability of completing the project, please input the targeted time and distribution mode(larger/smaller).")
        targeted = float(input("Targeted time: "))
        mode = input("Distribution mode(larger/smaller or l/s): ")
        if (mode != "larger" and mode != "smaller" and mode != "l" and mode != "s"):
                print_separator()
                print("Error input! Please input again!")
                print_separator()
                continue
        PERT_graph.probability(targeted, mode)
        print_separator()
    elif choice == "9":
        if loading_test(1):
            PERT_graph.initization()
            print("Activity graph:")
            print("Notice: If you don't have some fonts installed, you may get an error, just ignore it.")
            PERT_graph.graph_table()
            print_separator()
    elif choice == "10":
        print("Thank you! Goodbye!")
        break
    else:
        print_separator()
        print("Error input! Please input again!")
        print_separator()

```


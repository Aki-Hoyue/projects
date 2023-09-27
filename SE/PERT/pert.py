import os
import json
import math
import scipy.stats as st
import networkx as nx
import matplotlib.pyplot as plt
import queue
from tabulate import tabulate


# Define an array to store data and each item is a dictionary
PERT = []


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


# PERT graph
class Graph:
    def __init__(self):
        self.graph = {}

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

    # Calculate the earliest start time and earliest finish time
    def earliest_time(self):
        # Initialize the earliest start time and earliest finish time
        for i in self.graph:
            if (self.graph[i]["before_activity"] == []):
                self.graph[i]["earliest_finish_time"] = self.graph[i]["estimate_during"]
        # Calculate the earliest start time and earliest finish time
        for i in self.graph:
            for j in self.graph[i]["next_activity"]:
                if (self.graph[j]["earliest_start_time"] < self.graph[i]["earliest_finish_time"]):
                    self.graph[j]["earliest_start_time"] = self.graph[i]["earliest_finish_time"]
                self.graph[j]["earliest_finish_time"] = self.graph[j]["earliest_start_time"] + self.graph[j]["estimate_during"]

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


    # Calculate the slack time LS-ES
    def slack_time(self):
        # Calculate the slack time
        for i in self.graph:
            self.graph[i]["slack_time"] = self.graph[i]["latest_start_time"] - self.graph[i]["earliest_start_time"]


    # Calculate the critical path that Slack = 0
    def critical_path(self):
        # Calculate the critical path
        for i in self.graph:
            if (self.graph[i]["slack_time"] == 0):
                self.graph[i]["critical_path"] = True

    # Initialization before printing
    def initization(self):
        self.earliest_time()
        self.latest_time()
        self.slack_time()
        self.critical_path()

    # Print the Estimate duration
    def print_estimate_duration(self):
        print("Estimate duration: ")
        Estimate_table = [["Activity", "Estimate Duration", "Variance"]]
        for i in self.graph:
            Estimate_table.append([self.graph[i]["activity_code"], self.graph[i]["estimate_during"], self.graph[i]["variance"]])
        print(tabulate(Estimate_table, headers="firstrow", tablefmt='fancy_grid', numalign="center"))

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

    # Print the critical path
    def print_critical_path(self):
        print("Critical path: ")
        for i in self.graph:
            if (self.graph[i]["critical_path"]):
                print(self.graph[i]["activity_code"], end=" ")
                if (self.graph[i]["next_activity"] != []):
                    print("->", end=" ")
        print()

    # Print the expected project completion time
    def print_expected_project_completion_time(self):
        keys = list(self.graph.keys())
        print("Expected project completion time: %s" % self.graph[keys[-1]]["latest_finish_time"])

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
                    if (j in G.nodes()):
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
        dashed_edges = [(u, v) for u, v in G.edges if (u, v) in dash]
        nx.draw_networkx_edges(G, pos, edgelist=dashed_edges, style='dashed')
        solid_edges = [(u, v) for u, v in G.edges if (u, v) not in dash]
        nx.draw_networkx_edges(G, pos, edgelist=solid_edges, style='solid')
        nx.draw_networkx_edge_labels(G, pos, edge_labels=nx.get_edge_attributes(G, 'name'), font_family="sans-serif", font_size=10)
        plt.show()


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

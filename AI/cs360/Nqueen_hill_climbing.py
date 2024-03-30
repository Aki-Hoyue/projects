"""
Author: Hoyue
Date: 2024-03-30
"""

def hill_climbing(map):
    """
    Hill Climbing algorithm for solving the N-Queens problem.

    Parameters:
    - map (list of lists): The input map representing the chessboard. Each element in the map represents a column, and the elements within each column represent the number of queens that can be placed in that column.

    Returns:
    - list: A list of solutions to the N-Queens problem. Each solution is represented as a list of coordinates (row, column) indicating the positions of the queens on the chessboard.

    Algorithm:
    - Initialize a minNode object to store the minimum values and indices of each column.
    - Iterate over each column in the map.
    - Find the minimum value and its corresponding indices in each column.
    - Store the minimum values and indices in the minNode object.
    - Initialize an empty list to store the solutions.
    - Initialize a counter to keep track of the number of conflicts.
    - Find the minimum value among all the minimum values.
    - Iterate over each column with the minimum value.
    - For each column, iterate over its minimum indices.
    - Generate all possible solutions by placing queens at each minimum index.
    - Return the list of solutions.
    """
    
    class minNode:
        minEle = []
        minVal = []
        queen = []
    
    nextState = minNode()
    for col in map:
        min_val = 10  # Set maximum conflects number in the problem. 10 is only for 4-queens example.
        min_indices = []
        for i, element in enumerate(col):
            if element == "Q":
                nextState.queen.append(i)
                continue
            if element < min_val:
                min_val = element
                min_indices = [i]
            elif element == min_val:
                min_indices.append(i)
        nextState.minEle.append(min_indices)
        nextState.minVal.append(min_val)
    
    # print("queens",nextState.queen)

    ans = []
    count = 0
    nextmove = min(nextState.minVal)
    for i in range(len(nextState.queen)):
        count += conflect_detect(nextState.queen, i, nextState.queen[i])

    if nextmove >= int(count / 2):
        return []
    # print(nextmove, int(count / 2))
    
    for col, val in enumerate(nextState.minVal):
        nextqueen = nextState.queen.copy()
        if nextmove == val:  # col is i, row is k
            # print(col, nextState.minEle[col])
            for k in nextState.minEle[col]:
                nextqueen[col] = k
                ans.append(recaculate(nextqueen))
    
    # print(ans)
    # print("\n\n")
    return ans

def recaculate(queen):
    """
    Recalculate the values in the given `queen` array and return the updated map.

    Parameters:
        queen (list): A list representing the current configuration of the queen positions.

    Returns:
        list: The updated `nmap` array after recalculating the values.
    """
    
    nmap = [[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]]
    
    for i in range(len(queen)):
        nmap[i][queen[i]] = "Q"
    # print(queen)
    
    for col in range(len(map)):
        for row in range(len(map[col])):
            if nmap[col][row] == "Q":
                continue
            count = 0
            # print("recaculate {} {}".format(col, row))
            # print("now queen is", queen)
            
            #  Detect conflect for each cell in the chessboard. 
            count += conflect_detect(queen, col, row)
            
            nqueen = queen.copy()
            nqueen[col] = row
            # print("now nqueen is", nqueen)
            for i in range(len(queen)):
                if i == col: 
                    continue
                else:
                    count += conflect_detect(nqueen, i, queen[i])
            # print("All count is {}, true count is {}".format(count, int(count / 2)))
            # print()
            nmap[col][row] = int(count / 2)
    
    return nmap

def conflect_detect(queen, col, row):
    """
    A function to detect conflicts for a queen placed at a certain column and row. Not all the queens. Therefore, the solution of the funtion is doubled for each queen.
    
    Parameters:
    - queen (list): A list representing the positions of queens on the board.
    - col (int): The column in which the new queen is being placed.
    - row (int): The row in which the new queen is being placed.
    
    Returns:
    - count (int): The number of conflicts detected for the new queen placement.
    """
    
    count = 0
    for i in range(len(queen)):
        if i == col: 
            continue
        if queen[i] == row:
            # print("{} {} is Same row with Queen {} {}".format(col, row, i, queen[i]))
            count += 1
        elif col + row == 0 and i == queen[i]:
            # print("{} {} is First condition with Queen {} {}".format(col, row, i, queen[i]))
            count += 1
        elif col + row != 0 and abs(col - i) == abs(row - queen[i]):
            # print("{} {} is Same diagonal with Queen {} {}".format(col, row, i, queen[i]))
            count += 1
    return count

def dfs(start):
    """
    Perform depth-first search from the given start node.
    """
    
    print("\nStart from ", start)
    ans = hill_climbing(start)
    print("Now ans is ", ans)
    
    if ans.__len__() == 0:
        return
    else:
        for i in ans:
            dfs(i)
        print()

if __name__ == "__main__":
    map = [[4,"Q",3,2], ["Q",5,3,3], ["Q",5,3,3], [4,"Q",3,2]]  # 4-queens example
    print(map)
    dfs(map)

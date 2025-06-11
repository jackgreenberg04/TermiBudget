CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -O2
SRC=src/main.cpp src/User.cpp src/Transaction.cpp src/SavingsGoal.cpp

termbudget: $(SRC)
	$(CXX) $(CXXFLAGS) -o termbudget $(SRC)

clean:
	rm -f termbudget

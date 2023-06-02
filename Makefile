SRC = main.cpp server.cpp client.cpp database.cpp
OBJ = $(SRC:.cpp=.o)
EXE = chatUp
CPP = c++
CPPFLAGS = -std=c++11 -lsqlite3
RM = rm -f


all: $(EXE)

$(EXE): $(OBJ)
	$(CPP) $(CPPFLAGS) -o $(EXE) $(OBJ)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(EXE)

re: fclean all

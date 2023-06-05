INCLUDE = ${PWD}/includes/include 
LIB = ${PWD}/includes/lib
SRC = main.cpp server.cpp client.cpp database.cpp
OBJ = $(SRC:.cpp=.o)
EXE = chatUp
CPP = c++
CPPFLAGS = -std=c++11 -lsqlite3 -I$(INCLUDE) -L$(LIB) -lsodium

RM = rm -f


all: $(EXE)

$(EXE): $(OBJ)
	$(CPP) $(CPPFLAGS) -o $(EXE) $(OBJ)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

install-dependencies:
	# Install libsodium
	mkdir -p includes && cd libsodium-1.0.18 && ./configure --prefix=$(PWD)/includes && make && make install && cd ..

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(EXE)
drop:
	rm -rf ChatUp.db

re: fclean all

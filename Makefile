all: my-router.cpp
	g++ -o my-router my-router.cpp
clean:
	$(RM) my-router

all:
	g++ diff.cpp -std=c++17 -o diff
	g++ patch.cpp -std=c++17 -o patch

clean:
	rm diff patch

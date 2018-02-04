vsjson_test: src/vsjson.c src/vsjson_test.c
	gcc -Wall src/vsjson.c src/vsjson_test.c -o vsjson_test

clean:
	rm -f ./vsjson_test

check: vsjson_test
	./vsjson_test

memcheck: vsjson_test
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --error-exitcode=1 ./vsjson_test

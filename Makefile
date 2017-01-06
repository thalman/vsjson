vsjson_test: src/vsjson.c src/vsjson_test.c
	gcc -Wall src/vsjson.c src/vsjson_test.c -o vsjson_test

clean:
	rm -f ./vsjson_test

check: vsjson_test
	./vsjson_test

memcheck: vsjson_test
	valgrind ./vsjson_test

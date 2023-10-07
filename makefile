PARSER=parser
DEBUG=debug
HTTP_SERVER=http_server
cc=g++

.PHONY:all
all:$(PARSER) $(DEBUG) $(HTTP_SERVER)

$(PARSER):parser.cc
	$(cc) -o $@ $^ -lboost_system -lboost_filesystem -std=c++11 
$(DEBUG):debug.cc
	$(cc) -o $@ $^ -ljsoncpp -std=c++11 -lboost_system -lboost_filesystem -lpthread
$(HTTP_SERVER):http_server.cc
	$(cc) -o $@ $^ -ljsoncpp -lpthread -std=c++11 -lboost_system -lboost_filesystem 


.PHONY:clean
clean:
	rm -f $(PARSER) $(DEBUG) $(HTTP_SERVER)

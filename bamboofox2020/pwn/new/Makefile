server.out.local: server.o
	@echo '===> Linking'
	@ld server.o -o server.out.local

server.o: server.asm utils.asm socket.asm http.asm log.asm
	@echo '===> Assembling'
	@nasm -f elf64 server.asm

run: server.out
	@echo '===> Running'
	@./server.out

clean:
	@-rm -f *.o *.out

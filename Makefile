all:
	gcc -o post post.c `pkg-config libsoup-2.4 --libs --cflags`

clean:
	rm post

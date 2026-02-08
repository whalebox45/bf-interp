CC := gcc

%: %.c
	$(CC) $< -o $@
LINKER = gcc 
CC = gcc 

CFLAGS =
LFLAGS = -Wall

OBJDIR = ../obj


TARGET: $(OBJDIR) $(OBJDIR)/FTPclient $(OBJDIR)/FTPserver

$(OBJDIR)/FTPclient: $(OBJDIR)/FTPclient.o
	$(LINKER) $(LFLAGS) -o $@ $<

$(OBJDIR)/FTPserver: $(OBJDIR)/FTPserver.o
	$(LINKER) $(LFLAGS) -o $@ $<


$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJDIR):
	@mkdir -p $@
clean:
	rm $(OBJDIR)/*.o $(OBJDIR)/FTPclient $(OBJDIR)/FTPserver

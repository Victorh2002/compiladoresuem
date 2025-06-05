CC = gcc -g  
BIN= teste
FONTELEXICO = lex.yy.c
FONTETESTELEXICO = bison.tab.c bison.tab.h
all: $(BIN)
	@echo "Arquivos compilados!"
bison.tab.c: bison.y
	bison -H bison.y -Wcounterexamples
lex.yy.c: lexico.lex $(FONTETESTELEXICO)
	flex lexico.lex
teste: $(FONTELEXICO) 
	$(CC) -o teste $(FONTELEXICO) $(FONTETESTELEXICO)
clean:
	rm -vf $(BIN) $(FONTELEXICO) $(FONTETESTELEXICO)


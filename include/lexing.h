#ifndef LEXING_H
# define LEXING_H

# include "B_header.h"

void	handle_declaration(t_g_var_list	**head, t_statement_list *list, bool extrn);
void	yyerror(const char *s);


#endif
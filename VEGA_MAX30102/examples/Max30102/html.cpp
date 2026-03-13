#include "html.h"

// Como você está usando a variável html_page diretamente no servidor,
// o html.cpp pode ficar praticamente vazio. Mas deixo uma função auxiliar se quiser:

String getHTML() {
  return String(html_page);
}

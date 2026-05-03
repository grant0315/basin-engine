#include <basin/engine.h>
#include "editor_app.h"

int main() {
  basin::Engine engine(1400, 900, "Basin Editor");
  basin::EditorApp editor;
  engine.run(&editor);
  return 0;
}

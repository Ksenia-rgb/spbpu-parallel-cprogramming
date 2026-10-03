import sys
from clang.cindex import Index, CursorKind

#argv[1]  empty .clang-tidy file for write
#argv[2]  general .clang-tidy file
#argv[3]  camelBack case .clang-format file
#argv[4]  lower_case case .clang-format file
#argv[5:] files for fotmatting

def find_variables_ast_case(filename) -> int:
  index = Index.create()
  translation_unit = index.parse(filename)

  case = 0

  def visit_node(node):
    nonlocal case

    if node.kind == CursorKind.VAR_DECL or \
      node.kind == CursorKind.FIELD_DECL or \
      node.kind == CursorKind.PARM_DECL:
        name = node.spelling
        if name.lower() == name and "_" in name:
          case -= 1
        elif name.lower() != name and "_" not in name:
          case += 1

    for child in node.get_children():
      visit_node(child)

  visit_node(translation_unit.cursor)
  return case

def add_need_case():
  files = sys.argv
  tidy_write = open(files[1], mode='w+')
  tidy_general = open(files[2], mode='r')
  tidy_camel = open(files[3], mode='r')
  tidy_lower = open(files[4], mode='r')

  line = tidy_general.readline()
  while line and not ("CheckOptions" in line):
    tidy_write.write(line)
    line = tidy_general.readline()
  tidy_write.write(line)

  case = 0
  for file in files[5:]:
    case += find_variables_ast_case(file)

  if case >= 0:
    tidy_case = tidy_camel
  else:
    tidy_case = tidy_lower

  line_case = tidy_case.readline()
  while (line_case and not ("CheckOptions" in line_case)):
    line_case = tidy_case.readline()
  tidy_write.write(tidy_case.read())

  tidy_write.write(tidy_general.read())

  tidy_write.close()
  tidy_general.close()
  tidy_camel.close()
  tidy_lower.close()
  return

def main():
  RED = '\033[1;31m'
  RESET = '\033[0m'

  if len(sys.argv) <= 5:
    print(f'{RED}error:{RESET}', "no files for formatting")
    return 0

  add_need_case()
  return

if __name__ == "__main__":
  main()

import sys

#argv[1]  empty .clang-format file for write
#argv[2]  general .clang-format file
#argv[3]  break parentheses .clang-format file
#argv[4]  egypt parentheses .clang-format file
#argv[5:] files for fotmatting

def find_parentheses_style() -> int:
  files = sys.argv
  indent = 0
  control_tokens = ["if", "for", "do", "while", "case", "try", "catch", "class", "struct", "namespace"]
  for file_path in files[5:]:
    file = open(file_path, 'r')
    line = file.readline()
    while line:
      if any(token in line for token in control_tokens):
        if "{" in line:
          indent -= 1
        else:
          indent += 1
      line = file.readline()
    file.close()
  return indent

def add_need_style():
  files = sys.argv
  format_write = open(files[1], mode='w+')
  format_general = open(files[2], mode='r')
  format_break = open(files[3], mode='r')
  format_egypt = open(files[4], mode='r')

  line = format_general.readline()
  while line and not ("BraceWrapping" in line):
    format_write.write(line)
    line = format_general.readline()
  format_write.write(line)


  indent = find_parentheses_style()
  if indent >= 0:
    format_parentheses = format_break
  else:
    format_parentheses = format_egypt

  line_pars = format_parentheses.readline()
  while (line_pars and not ("BraceWrapping" in line_pars)):
    line_pars = format_parentheses.readline()
  format_write.write(format_parentheses.read())

  format_write.write(format_general.read())

  format_write.close()
  format_general.close()
  format_break.close()
  format_egypt.close()
  return

def main():
  RED = '\033[1;31m'
  RESET = '\033[0m'

  if len(sys.argv) <= 5:
    print(f'{RED}error:{RESET}', "no files for formatting")
    return 0

  add_need_style()
  return

if __name__ == "__main__":
  main()

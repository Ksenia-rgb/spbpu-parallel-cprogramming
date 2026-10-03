import sys

#argv[1]  --check option
#argv[2:] files for fotmatting

def get_header_guard_name(file_path):
  folder_index = file_path.rfind("/")
  if folder_index == -1:
    right_name = file_path
  else:
    right_name = file_path[folder_index + 1::]

  res_name = "" + right_name[0].upper()
  for i in range(1, len(right_name), 1):
    if (right_name[i] == '-' or right_name[i] == '.'):
      res_name += '_'
    elif (right_name[i - 1].isalpha() and right_name[i - 1].islower() and right_name[i].isalpha() and right_name[i].isupper()):
      res_name += '_'
      res_name += right_name[i].upper()
    else:
      res_name += right_name[i].upper()

  return res_name


def check_header_guard():
  return_code = 0

  files = sys.argv

  for file_path in files[2:]:
    file = open(file_path)

    if file_path.rfind(".hpp") != -1 or file_path.rfind(".h") != -1:
      header_guard_name = get_header_guard_name(file_path)
      line = file.readline()
      if line != "#ifndef " + header_guard_name + '\n':
        return_code = 1
        print_error(file_path, 1, header_guard_name)    
    file.close()

  return return_code

def print_error(file_path, str_count, name):
  RED = '\033[1;31m'
  BOLD = '\033[1;37m'
  RESET = '\033[0m'

  print(f'{BOLD}{file_path}:{str(str_count)}:', f'{RED}error:{RESET}',
        f'{BOLD}header guard is missing or does not conform to the required style: {name}{RESET}')
  return

def main():
  RED = '\033[1;31m'
  RESET = '\033[0m'

  files = sys.argv
  if len(files) <= 2:
    print(f'{RED}error:{RESET}', "no files for formatting")
    return 0
  if files[1] != "--check":
    print(f'{RED}error:{RESET}', "incorrect option")
    return 1

  if check_header_guard():
    sys.exit(1)
  return 0

if __name__ == "__main__":
  main()

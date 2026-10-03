import sys

#argv[1]  --check / --fix option
#argv[2:] files for fotmatting

def check_std_spaces():
  return_code = 0

  files = sys.argv

  for file_path in files[2:]:
    str_count = 1
    file = open(file_path, 'r')
    line = file.readline()
    while line:
      if "std ::" in line:
        return_code = 1
        print_warning(file_path, str_count)

      line = file.readline()
      str_count += 1
    file.close()
  return return_code

def fix_std_spaces():
  files = sys.argv

  for file_path in files[2:]:
    temp_file = []
    file = open(file_path, 'r+')
    line = file.readline()
    while line:
      if "std ::" in line:
        line = line.replace("std ::", "std::")
      temp_file.append(line)
      line = file.readline()
    file.seek(0)
    file.truncate()
    file.writelines(temp_file)
    file.close()
  return

def print_warning(file_path, str_count):
  PURPLE = '\033[1;35m'
  BOLD = '\033[1;37m'
  RESET = '\033[0m'

  print(f'{BOLD}{file_path}:{str(str_count)}:{RESET}', f'{PURPLE}warning:{RESET}',
        f'{BOLD}extra whitespace after std{RESET}')
  return

def main():
  RED = '\033[1;31m'
  RESET = '\033[0m'

  files = sys.argv
  if len(files) <= 2:
    print(f'{RED}error:{RESET}', "no files for formatting")
    return 0
  if files[1] == "--check":
    if check_std_spaces():
      sys.exit(1)
    return 0
  elif files[1] == "--fix":
    fix_std_spaces()
    return 0
  else:
    print(f'{RED}error:{RESET}', "incorrect option")
    return 0

if __name__ == "__main__":
  main()

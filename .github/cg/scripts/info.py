def print_info():
  info = '''Three options are available for the target `format-<labid>`:
  --check
  --fix
  --info

Three options are available for the target `tidy-<labid>`:
  --check
  --info

CG points that will be checked and fixed by clang-format:
  1   2   3   4   5   11  12  13  14  15  16  17  18
  19  20  22  33  34  39  40  41  49  52  53  55  57

CG points that will be checked by clang-tidy:
  8   9   10  21  23  26  30  32  35  36  37  38  44  45  47  48  50

CG points that will be checked, but you should be carefull with them:
  9   10  22  35  50

CG points that are agreements and will remain on your conscience:
  6   7   24  25  27  28  29  31  42  43  46  51  54  56

Remember: automatic checks should help you format your code,
but they shouldn’t completely relieve you of this responsibility.
A lot of details and conventions still fall on your shoulders.
Check your code and monitor the automatic checks you use.
And may the odds be ever in your favor!'''
  print(info)
  return

def main():
  print_info()
  return

if __name__ == "__main__":
  main()

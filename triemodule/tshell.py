import trie
import getch

def readcountries():
  print "Populating countries..."
  f = open("outOfSchoolPrimaryAge.csv", "r")
  # Skip the header line
  country = f.readline()
  while (country != ""):
    countryline = f.readline()
    country = (countryline.split(','))[0]
    trie.populate(country);
  print "Countries have been populated."

def doit():
  trie.init()
  readcountries()
  ch = ''
  while ch != 'n':
    str = ""
    while ch != '\r':
      ch = getch.getch()
      str += ch
      print " "
      print "TYPED: ",
      print str
      print "-------- SUBSTRINGS:---------"
      allMatches = trie.subs(str)
      print "-----------------------------"
      print allMatches
      print "-----------------------------"
    print "Enter another string? [y/n] ",
    ch = getch.getch()
    print " "
    print " "


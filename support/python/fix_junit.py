import sys
import os

from optparse import OptionParser



# ----------------------------------------------------------------------------------------------------------------------
# 'main' function
def main( argv_ ):

  usage = "%prog [options]"
  parser = OptionParser( usage=usage )
  
  parser.add_option( "-f", "--file", help="the file to fix", type="string", action="store", dest="file" )
  
 # try:                                
  (options, args) = parser.parse_args()
  #except getopt.GetoptError:           
   # usage()                          
    #sys.exit(2) 
  cmdline_options = vars( options )
  filename = cmdline_options[ "file" ]
  
  print( "Patching xml version of junit file..." )  


  try:
    with open( filename, 'r') as f:
      lines = f.read().splitlines()
  except OSError as e:
     error( 'Could not read junit file.' )

  try:
    with open( filename, 'w') as f:
      for l in lines:
        l = l.replace( "xml version=\"1.1\"", "xml version=\"1.0\"" )
        f.write( l + "\n" )
  except OSError as e:
     error( 'Could not write junit file.' )

# ------------------------------------------------------------------------------------------------------------
# entry point
if __name__ == "__main__":
  main( sys.argv )  # throw away first cmdline arg
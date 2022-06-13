from zipfile import ZipFile
import xml.etree.ElementTree as ET
import sys

if len( sys.argv ) < 3:
    print( "script input... output " )
    sys.exit( 1 )

with open( sys.argv[ -1 ], "a" ) as out:
    for p in sys.argv[ 1 : -1 ]:
        with ZipFile( p ) as z:

            # For all XML files
            for name in z.namelist( ):
                if name[ -6 : ] == ".xhtml":
                    print( name )
                    with z.open( name ) as infile:
                        root = ET.parse( infile )

                        # For all paragraph elements
                        for child in root.findall( ".//{*}p" ):
                            print( "!" )

                            # Add text to output
                            if child.text is not None:
                                out.write( child.text + '\n' )
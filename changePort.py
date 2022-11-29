#read input file
fin = open("Serveur/server2.h", "rt")
fin2 = open("Client/client2.h", "rt")
#read file contents to string
data = fin.read()
data2 = fin2.read()
#replace all occurrences of the required string
if "#define PORT 2007" in data:
    data = data.replace('#define PORT 2007', '#define PORT 2008')
    data2 = data2.replace('#define PORT 2007', '#define PORT 2008')
elif "#define PORT 2008" in data:
    data = data.replace('#define PORT 2008', '#define PORT 2009')
    data2 = data2.replace('#define PORT 2008', '#define PORT 2009')
elif "#define PORT 2009" in data:
    data = data.replace('#define PORT 2009', '#define PORT 2007')
    data2 = data2.replace('#define PORT 2009', '#define PORT 2007')

#close the input file
fin.close()
fin2.close()
#open the input file in write mode
fin = open("Serveur/server2.h", "wt")
fin2 = open("Client/client2.h", "wt")
#overrite the input file with the resulting data
fin.write(data)
fin2.write(data2)
#close the file
fin.close()
fin2.close()

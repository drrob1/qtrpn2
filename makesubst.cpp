#include "macros.h"

/* REVISION HISTORY
 * ----------------
 *
 * 28 Jan 20 -- Starting writing first version
 *
 *
 *
 */


RETURN string FUNCTION makesubst(string str) {
  string s;
  size_t n;

  s = str;
  for (n = s.find('='); n != string::npos; n = s.find('=')) {
    s = s.replace(n,1,"+");
  }

  for (n = s.find(';'); n != string::npos; n = s.find(';')) {
    s = s.replace(n,1,"*");
  }
  return s;
  
}


/*
 *
 * {{{
Find & Replace all sub strings – using Boost::replace_all
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
int main()
{
std::string data = "Boost Library is simple C++ Library";
std::cout<<data<<std::endl;
// Replace all occurrences of 'LIB' with 'XXX'
// Case Sensitive Version
boost::replace_all(data, "Lib", "XXX");
std::cout<<data<<std::endl;
}
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
 
 
 
int main() {
	std::string data = "Boost Library is simple C++ Library";
 
	std::cout<<data<<std::endl;
 
	// Replace all occurrences of 'LIB' with 'XXX'
	// Case Sensitive Version
	boost::replace_all(data, "Lib", "XXX");
 
	std::cout<<data<<std::endl;
 
 
}


Find & Replace all Case Insensitive Sub Strings using Boost::ireplace_all
#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
int main() {
std::string data = "Boost Library is simple C++ Library";
std::cout<<data<<std::endl;
// Replace all occurrences of 'LIB' with 'XXX'
// Case Insensitive Version
boost::ireplace_all(data, "LIB", "XXX");
std::cout<<data<<std::endl;
}



#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
 
 
 
int main()
{
	std::string data = "Boost Library is simple C++ Library";
 
	std::cout<<data<<std::endl;
 
	// Replace all occurrences of 'LIB' with 'XXX'
	// Case Insensitive Version
	boost::ireplace_all(data, "LIB", "XXX");
 
	std::cout<<data<<std::endl;
 
 
}
 Demming, Robert & Duffy, Daniel J. (2010). Introduction to the Boost C++ Libraries. Volume 1 - Foundations. Datasim. ISBN 978-94-91028-01-4.

Demming, Robert & Duffy, Daniel J. (2012). Introduction to the Boost C++ Libraries. Volume 2 - Advanced Libraries. Datasim. ISBN 978-94-91028-02-1.

Mukherjee, Arindam (2015). Learning Boost C++ Libraries. Packt. ISBN 978-1-78355-121-7.

Polukhin, Antony (2013). Boost C++ Application Development Cookbook. Packt. ISBN 978-1-84951-488-0.

Polukhin, Antony (2017). Boost C++ Application Development Cookbook (2 ed.). Packt. ISBN 978-1-78728-224-7.

Schäling, Boris (2011). The Boost C++ Libraries. XML Press. ISBN 978-0-9822191-9-5.

Schäling, Boris (2014). The Boost C++ Libraries (2 ed.). XML Press. p. 570. ISBN 978-1-937434-36-6.

Siek, Jeremy G.; Lee, Lie-Quan & Lumsdaine, Andrew (2001). The Boost Graph Library: User Guide and Reference Manual. Addison-Wesley. ISBN 978-0-201-72914-6.*




You can use apt-get command (requires sudo)

sudo apt-get install libboost-all-dev
Or you can call

aptitude search boost
find packages you need and install them using the apt-get command.



Get the version of Boost that you require. This is for 1.55 but feel free to change or manually download yourself:

wget -O boost_1_55_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download
tar xzvf boost_1_55_0.tar.gz
cd boost_1_55_0/
Get the required libraries, main ones are icu for boost::regex support:

sudo apt-get update
sudo apt-get install build-essential g++ python-dev autotools-dev libicu-dev build-essential libbz2-dev libboost-all-dev
Boost's bootstrap setup:

./bootstrap.sh --prefix=/usr/
Then build it with:

./b2
and eventually install it:

sudo ./b2 install





An update for Windows 10 Ubuntu Application via Subsystem (also works on standard Ubuntu):

You might have problems finding the package. If you do, never fear! PPA is here!

sudo add-apt-repository ppa:boost-latest/ppa
sudo apt-get update
Then run:

sudo apt-get install libboost-all-dev



https://theboostcpplibraries.com/introduction-installation
}}}
 *
 */



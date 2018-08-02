#include <sstream>

#include "ClueTest.h"
#include "DexLoader.h"

DexClasses& ClueTest::load_classes(const std::string& dexfile)
{
	DexMetadata dm;

	std::ostringstream dex_id;
	dex_id << "classes";
	if(stores.size()>0)
		dex_id << stores.size()+1;
	std::string dexid = dex_id.str();
	dm.set_id(dexid);

	std::cout << "Metadata: " << dm.get_id() << std::endl;

	DexStore store(dm);
	DexClasses classes = load_classes_from_dex(dexfile.c_str());
	store.add_classes(std::move(classes));
	stores.emplace_back(std::move(store));

	DexClasses& cls = stores.back().get_dexen().back();
	std::cout << "Loaded classes: " << cls.size() << std::endl ;
	return cls;
}


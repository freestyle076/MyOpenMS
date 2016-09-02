### Files from IMSLIB
### the directory name
set(directory include/OpenMS/CHEMISTRY/MASSDECOMPOSITION/IMS)

### list all header files of the directory here
set(sources_list_h
IMSAlphabet.h
IntegerMassDecomposer.h
MassDecomposer.h
RealMassDecomposer.h
Weights.h
IMSElement.h
IMSIsotopeDistribution.h
IMSAlphabetParser.h
IMSAlphabetTextParser.h
)


### add path to the filenames
set(sources_h)
foreach(i ${sources_list_h})
	list(APPEND sources_h ${directory}/${i})
endforeach(i)

### source group definition
source_group("Header Files\\OpenMS\\CHEMISTRY\\MASSDECOMPOSITION\\IMS" FILES ${sources_h})

set(OpenMS_sources_h ${OpenMS_sources_h} ${sources_h})


from libcpp cimport bool
from Types cimport *
from String cimport *
from ResidueModification cimport *

cdef extern from "<OpenMS/CHEMISTRY/ModificationDefinition.h>" namespace "OpenMS":
    
    cdef cppclass ModificationDefinition "OpenMS::ModificationDefinition":
        ModificationDefinition() nogil except +
        ModificationDefinition(ModificationDefinition) nogil except +
        ModificationDefinition(String &mod) nogil except +
        # void setTermSpecificity(ResidueModification::TermSpecificity pos) nogil except +
        # ResidueModification::TermSpecificity getTermSpecificity() nogil except +
        void setFixedModification(bool fixed) nogil except +
        bool isFixedModification() nogil except +
        void setMaxOccurences(UInt num) nogil except +
        UInt getMaxOccurences() nogil except +
        String getModification() nogil except +
        void setModification(String &modification) nogil except +
        bool operator==(ModificationDefinition &rhs) nogil except +
        bool operator!=(ModificationDefinition &rhs) nogil except +
        bool operator<(ModificationDefinition &) nogil except +

        void setTermSpecificity(TermSpecificity pos) nogil except +
        TermSpecificity getTermSpecificity() nogil except +

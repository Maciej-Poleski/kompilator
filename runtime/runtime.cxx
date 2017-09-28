#include <cstdint>
#include <memory>
#include <vector>
#include <deque>
#include <cassert>
#include <cstring>
#include <unordered_set>
#include <unordered_map>

#include <iostream>
#include <limits>

using UnknownBlob = uint8_t;

// The most significant bit of type_id is set when value of represented term is set
static constexpr uint8_t SetTypeIdMarker = 0x80;
static constexpr uint8_t AnnotationTypeIdMarker = 0x40;

static constexpr uint8_t IntTypeId = 1;
struct IntRepresentation
{
    uint8_t type_id = IntTypeId;
    uint32_t ref_count = 0;
    int32_t value;
};

static constexpr uint8_t BoolTypeId = 2;
struct BoolRepresentation
{
    uint8_t type_id = BoolTypeId;
    uint32_t ref_count = 0;
    uint8_t value;
};

static constexpr uint8_t PtrTypeId = 3;
struct PtrRepresentation
{
    uint8_t type_id = PtrTypeId;
    uint32_t ref_count = 0;
    UnknownBlob *value;
    uint8_t pointee_type_ids[0];  // possibly annotated, set is not encoded here
};

static constexpr uint8_t RecordTypeId = 4;
struct RecordRepresentation
{
    uint8_t type_id = SetTypeIdMarker | RecordTypeId;
    uint32_t ref_count = 0;
    uint32_t num_of_fields;
    uint32_t record_name_length;
    char record_name[0];
//    Field1Type field_1;
//    ...
//    FieldnType field_n;
};

static constexpr uint8_t FunctionSymbolTypeId = 5;
struct FunctionSymbolRepresentation
{
    uint8_t type_id = SetTypeIdMarker | FunctionSymbolTypeId;
    uint32_t ref_count = 0;
    uint32_t num_of_fields;
    uint32_t symbol_name_length;
    char symbol_name[0];
//    Subterm1Type subterm_1;
//    ...
//    SubtermnType subterm_n;
};

static constexpr uint8_t VariableTypeId = 6;
struct VariableRepresentation
{
    uint8_t type_id = VariableTypeId;
    uint32_t ref_count = 0;
    UnknownBlob *value = nullptr;
};

class LogEntry
{
public:
    virtual ~LogEntry() = default;

    virtual void revert() = 0;
};

class ReplaceMemoryRegion : public LogEntry
{
public:
    ReplaceMemoryRegion(void *location, uint32_t size);

    void revert() override;

private:
    void *location;
    std::vector<std::uint8_t> data;
};

ReplaceMemoryRegion::ReplaceMemoryRegion(void *location, uint32_t size)
        : location(location), data(reinterpret_cast<uint8_t *>(location), reinterpret_cast<uint8_t *>(location) + size)
{

}

void ReplaceMemoryRegion::revert()
{
    memcpy(location, data.data(), data.size());
}

template<typename T>
class ReplaceMemoryObject : public LogEntry
{
public:
    explicit ReplaceMemoryObject(T *object);

    void revert() override;

private:
    void *const location;
    std::array<std::uint8_t, sizeof(T)> data;
};

template<typename T>
ReplaceMemoryObject<T>::ReplaceMemoryObject(T *object)
        : location(object)
{
    memcpy(data.data(), object, sizeof(T));
}

template<typename T>
void ReplaceMemoryObject<T>::revert()
{
    memcpy(location, data.data(), sizeof(T));
}

template<typename T>
std::unique_ptr<ReplaceMemoryObject<T>> MakeReplaceMemoryObject(T *object)
{
    return std::make_unique<ReplaceMemoryObject<T>>(object);
}

class AllocateMemory : public LogEntry
{
public:
    explicit AllocateMemory(void *location);

    void revert() override;

private:
    void *location;
};

AllocateMemory::AllocateMemory(void *location)
        : location(location)
{
}

void AllocateMemory::revert()
{
    free(location);
}

static std::vector<std::unique_ptr<LogEntry>> sldLog;

extern "C" uint32_t _DI_getLogMarker()
{
    assert(sldLog.size() <= std::numeric_limits<uint32_t>::max());
    return uint32_t(sldLog.size());
}

extern "C" void _DI_rollbackToMarker(uint32_t marker)
{
    assert(marker <= sldLog.size());
    while (sldLog.size() > marker) {
        sldLog.back()->revert();
        sldLog.pop_back();
    }
    assert(sldLog.size() == marker);
}

static std::deque<std::tuple<bool (***)(UnknownBlob *, UnknownBlob *), UnknownBlob *>> sldGoalList;

class AddGoal : public LogEntry
{
public:
    explicit AddGoal() = default;

    void revert() override;
};

void AddGoal::revert()
{
    sldGoalList.pop_back();
}

class RemoveGoal : public LogEntry
{
public:
    explicit RemoveGoal(decltype(sldGoalList)::value_type goal);

    void revert() override;

private:
    decltype(sldGoalList)::value_type
    goal;
};

RemoveGoal::RemoveGoal(decltype(sldGoalList)::value_type goal)
        : goal(goal)
{}

void RemoveGoal::revert()
{
    sldGoalList.push_front(goal);
}

static uint8_t getType(UnknownBlob *representation)
{
    return *representation & uint8_t(0x3f);
}

static bool isSet(UnknownBlob *representation)
{
    return (*representation) >> 7;
}

static bool isAnnotated(UnknownBlob *representation)
{
    return (((*representation) >> 6) & 1) == 1;
}

static IntRepresentation *asInt(UnknownBlob *representation)
{
    assert(getType(representation) == IntTypeId);
    return reinterpret_cast<IntRepresentation *>(representation);
}

static BoolRepresentation *asBool(UnknownBlob *representation)
{
    assert(getType(representation) == BoolTypeId);
    return reinterpret_cast<BoolRepresentation *>(representation);
}

static PtrRepresentation *asPtr(UnknownBlob *representation)
{
    assert(getType(representation) == PtrTypeId);
    return reinterpret_cast<PtrRepresentation *>(representation);
}

static RecordRepresentation *asRecord(UnknownBlob *representation)
{
    assert(getType(representation) == RecordTypeId);
    return reinterpret_cast<RecordRepresentation *>(representation);
}

static FunctionSymbolRepresentation *asFunctionSymbol(UnknownBlob *representation)
{
    assert(getType(representation) == FunctionSymbolTypeId);
    return reinterpret_cast<FunctionSymbolRepresentation *>(representation);
}

static VariableRepresentation *asVariable(UnknownBlob *representation)
{
    assert(getType(representation) == VariableTypeId);
    return reinterpret_cast<VariableRepresentation *>(representation);
}

static UnknownBlob **getRecordFields(RecordRepresentation *recordRepresentation)
{
    return reinterpret_cast<UnknownBlob **>(recordRepresentation->record_name +
                                            recordRepresentation->record_name_length);
}

static UnknownBlob **getFunctionSymbolSubterms(FunctionSymbolRepresentation *functionSymbolRepresentation)
{
    return reinterpret_cast<UnknownBlob **>(functionSymbolRepresentation->symbol_name +
                                            functionSymbolRepresentation->symbol_name_length);
}

static unsigned sizeOfPointeeType(uint8_t *typeIterator)
{
    unsigned typeRepresentationSize = 0;
    while (getType(typeIterator) == PtrTypeId) {
        typeRepresentationSize += 1;
        typeIterator += 1;
    }
    const uint8_t lastType = getType(typeIterator);
    if (lastType == IntTypeId) {
        typeRepresentationSize += 1;
    } else if (lastType == BoolTypeId) {
        typeRepresentationSize += 1;
    } else if (lastType == VariableTypeId) {
        typeRepresentationSize += 1;
    } else if (lastType == RecordTypeId) {
        uint32_t *sizePtr = reinterpret_cast<uint32_t *>(typeIterator + 1);
        typeRepresentationSize += 1 + 4 + *sizePtr;
    } else {
        assert(false);
    }
    return typeRepresentationSize;
}

static unsigned sizeOfRepresentation(UnknownBlob *representation)
{
    auto type = getType(representation);
    if (type == IntTypeId) {
        return sizeof(IntRepresentation);
    } else if (type == BoolTypeId) {
        return sizeof(BoolRepresentation);
    } else if (type == PtrTypeId) {
        return sizeof(PtrRepresentation) + sizeOfPointeeType(asPtr(representation)->pointee_type_ids);
    } else if (type == VariableTypeId) {
        return sizeof(VariableRepresentation);
    } else if (type == RecordTypeId) {
        auto record = asRecord(representation);
        return sizeof(RecordRepresentation) + record->record_name_length +
               record->num_of_fields * sizeof(UnknownBlob *);
    } else if (type == FunctionSymbolTypeId) {
        auto functionSymbol = asFunctionSymbol(representation);
        return sizeof(FunctionSymbolRepresentation) + functionSymbol->symbol_name_length +
               functionSymbol->num_of_fields * sizeof(UnknownBlob *);
    } else {
        assert(false);
    }
}

static UnknownBlob *followSetVars(UnknownBlob *unknownBlob)
{
    assert(unknownBlob);
    if (isSet(unknownBlob) && (getType(unknownBlob) == VariableTypeId)) {
        return followSetVars(asVariable(unknownBlob)->value);
    }
    return unknownBlob;
}

static bool isUnsetVar(UnknownBlob *unknownBlob)
{
    return getType(followSetVars(unknownBlob)) == VariableTypeId;
}

static bool isCompound(UnknownBlob *unknownBlob)
{
    if (getType(unknownBlob) == RecordTypeId) {
        return true;
    }
    if (getType(unknownBlob) == FunctionSymbolTypeId) {
        return true;
    }
    return false;
}

static std::pair<UnknownBlob **, UnknownBlob **> getSubtermList(UnknownBlob *representation)
{
    assert(isCompound(representation));
    if (getType(representation) == RecordTypeId) {
        RecordRepresentation *recordRepresentation = asRecord(representation);
        UnknownBlob **begin = getRecordFields(recordRepresentation);
        return {begin, begin + recordRepresentation->num_of_fields};
    } else if (getType(representation) == FunctionSymbolTypeId) {
        FunctionSymbolRepresentation *functionSymbolRepresentation = asFunctionSymbol(representation);
        UnknownBlob **begin = getFunctionSymbolSubterms(functionSymbolRepresentation);
        return {begin, begin + functionSymbolRepresentation->num_of_fields};
    } else {
        assert(false);
    }
};

static bool hasInStructureImpl(UnknownBlob *what, UnknownBlob *structure, std::unordered_set<UnknownBlob *> &V)
{
    if (V.find(structure) != V.end()) {
        return false;
    }
    V.insert(structure);
    if (isCompound(structure)) {
        for (auto list = getSubtermList(structure); list.first != list.second; ++list.first) {
            if (hasInStructureImpl(what, followSetVars(*list.first), V)) {
                return true;
            }
        }
    }
    return false;
}

static bool hasInStructure(UnknownBlob *what, UnknownBlob *structure)
{
    assert(what != structure);
    assert(isUnsetVar(what));
    assert(!isUnsetVar(structure));
    std::unordered_set<UnknownBlob *> visited;
    return hasInStructureImpl(what, structure, visited);
}

static void recordInSubstitution(UnknownBlob *unset, UnknownBlob *data)
{
    assert(!isSet(unset));
//    assert(sizeOfRepresentation(unset) >= sizeof(VariableRepresentation));  // add padding in alloc
    sldLog.push_back(std::make_unique<ReplaceMemoryRegion>(unset, sizeOfRepresentation(unset)));
    bool annotated = isAnnotated(unset);
    // overwrite representation
    VariableRepresentation *var = reinterpret_cast<VariableRepresentation *>(unset);
    unset = nullptr;
    var->type_id = VariableTypeId | SetTypeIdMarker;
    if (annotated) {
        // preserve annotation for verifyAnnotation
        var->type_id |= AnnotationTypeIdMarker;
    }
    var->value = data;
}

static bool verifyAnnotationImpl(UnknownBlob *representation, std::unordered_map<UnknownBlob *, bool> &set)
{
    representation = followSetVars(representation);
    if (set.find(representation) != set.end()) {
        return set[representation];
    }
    if (isCompound(representation)) {
        bool termIsSet = true;
        for (auto list = getSubtermList(representation); list.first != list.second; ++list.first) {
            if (!verifyAnnotationImpl(*list.first, set)) {
                termIsSet = false;
            }
        }
        set[representation] = termIsSet;
        return termIsSet;
    } else if (getType(representation) == VariableTypeId) {
        if (isSet(representation)) {
            bool result = verifyAnnotationImpl(asVariable(representation)->value, set);
            set[representation] = result;
            return result;
        } else {
            set[representation] = false;
            return false;
        }
    } else {
        bool result = isSet(representation);
        set[representation] = result;
        return result;
    }
}

// true - annotations are honored in substitution
static bool verifyAnnotation(UnknownBlob *representation)
{
    if (!isAnnotated(representation)) {
        return true;
    }
    std::unordered_map<UnknownBlob *, bool> setState;
    return verifyAnnotationImpl(representation, setState);
}

extern "C" bool _DI_unify(UnknownBlob *lhs, UnknownBlob *rhs)
{
    using std::swap;
    assert(lhs);  // Or fail to hack _DI_invokeFunction
    assert(rhs);
    lhs = followSetVars(lhs);
    rhs = followSetVars(rhs);
    if (isUnsetVar(lhs) && isUnsetVar(rhs)) {
        if (lhs == rhs) {
            return true;
        } else {
            recordInSubstitution(lhs, rhs);
            return true;
        }
    } else if (isUnsetVar(lhs) && !isUnsetVar(rhs)) {
        assert(getType(rhs) != VariableTypeId);
        if (hasInStructure(lhs, rhs)) {
            return false;
        } else {
            recordInSubstitution(lhs, rhs);
            return true;
        }
    } else if (!isUnsetVar(lhs) && isUnsetVar(rhs)) {
        assert(getType(lhs) != VariableTypeId);
        if (hasInStructure(rhs, lhs)) {
            return false;
        } else {
            recordInSubstitution(rhs, lhs);
            return true;
        }
    } else {
        assert(getType(lhs) != VariableTypeId);
        assert(getType(rhs) != VariableTypeId);
        if (getType(lhs) != getType(rhs)) {
            return false;
        }
        if (getType(lhs) == PtrTypeId) {
            PtrRepresentation *lhsPtr = asPtr(lhs);
            PtrRepresentation *rhsPtr = asPtr(rhs);
            if (sizeOfPointeeType(lhsPtr->pointee_type_ids) != sizeOfPointeeType(rhsPtr->pointee_type_ids)) {
                return false;
            }
            if (memcmp(lhsPtr->pointee_type_ids, rhsPtr->pointee_type_ids,
                       sizeOfPointeeType(lhsPtr->pointee_type_ids)) != 0) {
                return false;
            }
        }
        if (!isSet(rhs)) {
            swap(lhs, rhs);
        }
        // unset unset | unset set | set set
        // compound types are always set
        if (!isSet(lhs)) {
            recordInSubstitution(lhs, rhs);
            return verifyAnnotation(lhs) && verifyAnnotation(rhs);
        }
        assert(isSet(lhs));
        assert(isSet(rhs));

        if (getType(lhs) == IntTypeId) {
            IntRepresentation *lhsi = asInt(lhs);
            IntRepresentation *rhsi = asInt(rhs);
            if (lhsi->value == rhsi->value) {
                return verifyAnnotation(lhs) && verifyAnnotation(rhs);
            } else {
                return false;
            }
        } else if (getType(lhs) == BoolTypeId) {
            BoolRepresentation *lhsb = asBool(lhs);
            BoolRepresentation *rhsb = asBool(rhs);
            if ((lhsb->value == 0) == (rhsb->value == 0)) {
                return verifyAnnotation(lhs) && verifyAnnotation(rhs);
            } else {
                return false;
            }
        } else if (getType(lhs) == PtrTypeId) {
            PtrRepresentation *lhsp = asPtr(lhs);
            PtrRepresentation *rhsp = asPtr(rhs);
            if (lhsp->value == rhsp->value) {
                return verifyAnnotation(lhs) && verifyAnnotation(rhs);
            } else {
                return false;
            }
        } else if (getType(lhs) == RecordTypeId) {
            RecordRepresentation *lhsr = asRecord(lhs);
            RecordRepresentation *rhsr = asRecord(rhs);
            if (lhsr->record_name_length != rhsr->record_name_length) {
                return false;
            }
            if (strncmp(lhsr->record_name, rhsr->record_name, lhsr->record_name_length) != 0) {
                return false;
            }
            assert(lhsr->num_of_fields == rhsr->num_of_fields);
            UnknownBlob **lhsFieldI = getRecordFields(lhsr);
            UnknownBlob **rhsFieldI = getRecordFields(rhsr);
            for (unsigned i = 0; i < lhsr->num_of_fields; ++i, ++lhsFieldI, ++rhsFieldI) {
                if (!_DI_unify(*lhsFieldI, *rhsFieldI)) {
                    return false;
                }
            }
            return verifyAnnotation(lhs) && verifyAnnotation(rhs);
        } else if (getType(lhs) == FunctionSymbolTypeId) {
            FunctionSymbolRepresentation *lhsf = asFunctionSymbol(lhs);
            FunctionSymbolRepresentation *rhsf = asFunctionSymbol(rhs);
            if (lhsf->symbol_name_length != rhsf->symbol_name_length) {
                return false;
            }
            if (strncmp(lhsf->symbol_name, rhsf->symbol_name, lhsf->symbol_name_length) != 0) {
                return false;
            }
            if (lhsf->num_of_fields != rhsf->num_of_fields) {
                return false;
            }
            UnknownBlob **lhsSubtermI = getFunctionSymbolSubterms(lhsf);
            UnknownBlob **rhsSubtermI = getFunctionSymbolSubterms(rhsf);
            for (unsigned i = 0; i < lhsf->num_of_fields; ++i, ++lhsSubtermI, ++rhsSubtermI) {
                if (!_DI_unify(*lhsSubtermI, *rhsSubtermI)) {
                    return false;
                }
            }
            return verifyAnnotation(lhs) && verifyAnnotation(rhs);
        } else {
            assert(false);
        }
    }
}

extern "C" IntRepresentation *_DI_allocateInt(bool annotated, bool set, int32_t value)
{
    IntRepresentation *representation = reinterpret_cast<IntRepresentation *>(malloc(
            std::max(sizeof(IntRepresentation), sizeof(VariableRepresentation))));
    sldLog.push_back(std::make_unique<AllocateMemory>(representation));
    *representation = {IntTypeId, 0, static_cast<int32_t>(0xCCCCCCCC)};
    if (annotated) {
        representation->type_id |= AnnotationTypeIdMarker;
    }
    if (set) {
        representation->type_id |= SetTypeIdMarker;
        representation->value = value;
    }
    return representation;
}

extern "C" BoolRepresentation *_DI_allocateBool(bool annotated, bool set, uint8_t value)
{
    BoolRepresentation *representation = reinterpret_cast<BoolRepresentation *>(malloc(
            std::max(sizeof(BoolRepresentation), sizeof(VariableRepresentation))));
    sldLog.push_back(std::make_unique<AllocateMemory>(representation));
    *representation = {BoolTypeId, 0, 0xCC};
    if (annotated) {
        representation->type_id |= AnnotationTypeIdMarker;
    }
    if (set) {
        representation->type_id |= SetTypeIdMarker;
        representation->value = value;
    }
    return representation;
}

extern "C" PtrRepresentation *_DI_allocatePtr(bool annotated, bool set, UnknownBlob *value, uint8_t *pointeeTypeId)
{
    auto pointeeTypeSize = sizeOfPointeeType(pointeeTypeId);
    PtrRepresentation *representation = reinterpret_cast<PtrRepresentation *>(malloc(
            std::max(sizeof(PtrRepresentation) + pointeeTypeSize, sizeof(VariableRepresentation))));
    sldLog.push_back(std::make_unique<AllocateMemory>(representation));
    *representation = {PtrTypeId, 0, reinterpret_cast<UnknownBlob *>(0xCCCCCCCCCCCCCCCC), {}};
    if (annotated) {
        representation->type_id |= AnnotationTypeIdMarker;
    }
    if (set) {
        representation->type_id |= SetTypeIdMarker;
        representation->value = value;
    }
    memcpy(representation->pointee_type_ids, pointeeTypeId, pointeeTypeSize);
    return representation;
}

extern "C" RecordRepresentation *
_DI_allocateRecord(bool annotated, uint32_t numOfFields, uint32_t nameLength, const char *recordName)
{
    RecordRepresentation *representation = reinterpret_cast<RecordRepresentation *>(malloc(
            std::max(sizeof(RecordRepresentation) + nameLength + numOfFields * sizeof(UnknownBlob *),
                     sizeof(VariableRepresentation))));
    sldLog.push_back(std::make_unique<AllocateMemory>(representation));
    *representation = {RecordTypeId | SetTypeIdMarker, 0, 0xCCCCCCCC, 0xCCCCCCCC, {}};
    if (annotated) {
        representation->type_id |= AnnotationTypeIdMarker;
    }
    representation->num_of_fields = numOfFields;
    representation->record_name_length = nameLength;
    memcpy(representation->record_name, recordName, nameLength);
    return representation;
    // Pointers to fields are NOT initialized here
}

extern "C" FunctionSymbolRepresentation *
_DI_allocateFunctionSymbol(bool annotated, uint32_t numOfSubterms, uint32_t nameLength, const char *symbolName)
{
    FunctionSymbolRepresentation *representation = reinterpret_cast<FunctionSymbolRepresentation *>(malloc(
            std::max(sizeof(FunctionSymbolRepresentation) + nameLength + numOfSubterms * sizeof(UnknownBlob *),
                     sizeof(VariableRepresentation))));
    sldLog.push_back(std::make_unique<AllocateMemory>(representation));
    *representation = {FunctionSymbolTypeId | SetTypeIdMarker, 0, 0xCCCCCCCC, 0xCCCCCCCC, {}};
    if (annotated) {
        representation->type_id |= AnnotationTypeIdMarker;
    }
    representation->num_of_fields = numOfSubterms;
    representation->symbol_name_length = nameLength;
    memcpy(representation->symbol_name, symbolName, nameLength);
    return representation;
    // Pointers to subterms are NOT initialized here
}

extern "C" VariableRepresentation *_DI_allocateVariable(bool annotated)
{
    VariableRepresentation *representation = reinterpret_cast<VariableRepresentation *>(malloc(
            sizeof(VariableRepresentation)));
    sldLog.push_back(std::make_unique<AllocateMemory>(representation));
    *representation = {VariableTypeId, 0};
    if (annotated) {
        representation->type_id |= AnnotationTypeIdMarker;
    }
    return representation;
    // Value is NOT initialized (variable is fresh)
}

extern "C" void _DI_assignRecordField(RecordRepresentation *recordRepresentation, UnknownBlob *field, uint32_t index)
{
    UnknownBlob **address = reinterpret_cast<UnknownBlob **>(
            recordRepresentation->record_name + recordRepresentation->record_name_length +
            index * sizeof(UnknownBlob *));
    sldLog.push_back(MakeReplaceMemoryObject(address));
    *address = field;
}

extern "C" void
_DI_assignFunctionSymbolSubterm(FunctionSymbolRepresentation *functionSymbol, UnknownBlob *subterm, uint32_t index)
{
    UnknownBlob **address = reinterpret_cast<UnknownBlob **>(
            functionSymbol->symbol_name + functionSymbol->symbol_name_length + index * sizeof(UnknownBlob *));
    sldLog.push_back(MakeReplaceMemoryObject(address));
    *address = subterm;
}

extern "C" int32_t _DI_getIntValue(UnknownBlob *representation)
{
    representation = followSetVars(representation);
    assert(getType(representation) == IntTypeId);
    assert(isSet(representation));
    IntRepresentation *intRepresentation = asInt(representation);
    return intRepresentation->value;
}

extern "C" bool _DI_getBoolValue(UnknownBlob *representation)
{
    representation = followSetVars(representation);
    assert(getType(representation) == BoolTypeId);
    assert(isSet(representation));
    BoolRepresentation *boolRepresentation = asBool(representation);
    return boolRepresentation->value;
}

extern "C" UnknownBlob *_DI_getPtrValue(UnknownBlob *representation)
{
    representation = followSetVars(representation);
    assert(getType(representation) == PtrTypeId);
    assert(representation);
    PtrRepresentation *ptrRepresentation = asPtr(representation);
    return ptrRepresentation->value;
}

extern "C" void _DI_setPtrValue(UnknownBlob *representation, UnknownBlob *newValue)
{
    assert(representation);
    assert(newValue);
    representation = followSetVars(representation);
    assert(getType(representation) == PtrTypeId);
    PtrRepresentation *ptrRepresentation = asPtr(representation);
    sldLog.push_back(MakeReplaceMemoryObject(&ptrRepresentation->value));
    ptrRepresentation->value = newValue;
    if (!isSet(representation)) {
        sldLog.push_back(MakeReplaceMemoryObject(representation));
        *representation |= SetTypeIdMarker;
    }
}

extern "C" UnknownBlob *_DI_getRecordMember(UnknownBlob *representation, uint32_t index)
{
    representation = followSetVars(representation);
    assert(getType(representation) == RecordTypeId);
    RecordRepresentation *recordRepresentation = asRecord(representation);
    UnknownBlob **address = reinterpret_cast<UnknownBlob **>(
            recordRepresentation->record_name + recordRepresentation->record_name_length +
            index * sizeof(UnknownBlob *));
    return *address;
}

extern "C" UnknownBlob *_DI_deepCopyRepresentation(UnknownBlob *representation)
{
    assert(representation);
    representation = followSetVars(representation);
    if (getType(representation) == IntTypeId) {
        IntRepresentation *intRepresentation = asInt(representation);
        IntRepresentation *clonedRepresentation = _DI_allocateInt(isAnnotated(representation), isSet(representation),
                                                                  intRepresentation->value);
        return reinterpret_cast<UnknownBlob *>(clonedRepresentation);
    } else if (getType(representation) == BoolTypeId) {
        BoolRepresentation *boolRepresentation = asBool(representation);
        BoolRepresentation *clonedRepresentation = _DI_allocateBool(isAnnotated(representation), isSet(representation),
                                                                    boolRepresentation->value);
        return reinterpret_cast<UnknownBlob *>(clonedRepresentation);
    } else if (getType(representation) == PtrTypeId) {
        PtrRepresentation *ptrRepresentation = asPtr(representation);
        PtrRepresentation *clonedRepresentation = _DI_allocatePtr(isAnnotated(representation), isSet(representation),
                                                                  ptrRepresentation->value,
                                                                  ptrRepresentation->pointee_type_ids);
        return reinterpret_cast<UnknownBlob *>(clonedRepresentation);
    } else if (getType(representation) == RecordTypeId) {
        RecordRepresentation *recordRepresentation = asRecord(representation);
        RecordRepresentation *clonedRepresentation = _DI_allocateRecord(isAnnotated(representation),
                                                                        recordRepresentation->num_of_fields,
                                                                        recordRepresentation->record_name_length,
                                                                        recordRepresentation->record_name);
        UnknownBlob **clonedI = getRecordFields(clonedRepresentation);
        UnknownBlob **sourceI = getRecordFields(recordRepresentation);
        for (unsigned i = 0; i < recordRepresentation->num_of_fields; ++i, ++clonedI, ++sourceI) {
            *clonedI = _DI_deepCopyRepresentation(*sourceI);
        }
        return reinterpret_cast<UnknownBlob *>(clonedRepresentation);
    } else if (getType(representation) == FunctionSymbolTypeId) {
        FunctionSymbolRepresentation *functionSymbolRepresentation = asFunctionSymbol(representation);
        FunctionSymbolRepresentation *clonedRepresentation = _DI_allocateFunctionSymbol(isAnnotated(representation),
                                                                                        functionSymbolRepresentation->num_of_fields,
                                                                                        functionSymbolRepresentation->symbol_name_length,
                                                                                        functionSymbolRepresentation->symbol_name);
        UnknownBlob **clonedI = getFunctionSymbolSubterms(clonedRepresentation);
        UnknownBlob **sourceI = getFunctionSymbolSubterms(functionSymbolRepresentation);
        for (unsigned i = 0; i < functionSymbolRepresentation->num_of_fields; ++i, ++clonedI, ++sourceI) {
            *clonedI = _DI_deepCopyRepresentation(*sourceI);
        }
        return reinterpret_cast<UnknownBlob *>(clonedRepresentation);
    } else if (getType(representation) == VariableTypeId) {
        assert(!isSet(representation));
        VariableRepresentation *clonedVariable = _DI_allocateVariable(isAnnotated(representation));
        return reinterpret_cast<UnknownBlob *>(clonedVariable);
    } else {
        assert(false);
    }
}

extern "C" bool
_DI_invoke(bool (***functions)(UnknownBlob *, UnknownBlob *), UnknownBlob *callHead, UnknownBlob *callResult)
{
    for (auto blockPtr = functions; *blockPtr; ++blockPtr) {
        for (auto blockIterator = *blockPtr; *blockIterator; ++blockIterator) {
            if ((*blockIterator)(callHead, callResult)) {
                return true;
            }
        }
    }
    return false;
}

extern "C" bool _DI_continueSldResolution()
{
    // loop is not necessary
    while (!sldGoalList.empty()) {
        bool (***functions)(UnknownBlob *, UnknownBlob *);
        UnknownBlob *goal;
        std::tie(functions, goal) = sldGoalList.front();
        sldLog.push_back(std::make_unique<RemoveGoal>(sldGoalList.front()));
        sldGoalList.pop_front();
        UnknownBlob *unusedResult = reinterpret_cast<UnknownBlob *>(_DI_allocateVariable(false));
        if (!_DI_invoke(functions, goal, unusedResult)) {
            // no need to rollback on failure, caller will handle that
            return false;
        }
    }
    return true;
}

extern "C" void _DI_failed()
{
    // TODO: Add predicate name
    std::cerr << "Predicate failed\n";
    abort();
}

extern "C" bool _DI_equals(UnknownBlob *lhs, UnknownBlob *rhs)
{
    assert(lhs);
    assert(rhs);
    lhs = followSetVars(lhs);
    rhs = followSetVars(rhs);
    if (getType(lhs) != getType(rhs)) {
        return false;
    }
    auto type = getType(lhs);
    if (type == IntTypeId) {
        if (isSet(lhs) != isSet(rhs)) {
            return false;
        }
        if (!isSet(lhs)) {
            return true;
        }
        IntRepresentation *lhsi = asInt(lhs);
        IntRepresentation *rhsi = asInt(rhs);
        if (lhsi->value != rhsi->value) {
            return false;
        }
        return true;
    } else if (type == BoolTypeId) {
        if (isSet(lhs) != isSet(rhs)) {
            return false;
        }
        if (!isSet(lhs)) {
            return true;
        }
        BoolRepresentation *lhsb = asBool(lhs);
        BoolRepresentation *rhsb = asBool(rhs);
        if (lhsb->value != rhsb->value) {
            return false;
        }
        return true;
    } else if (type == PtrTypeId) {
        if (isSet(lhs) != isSet(rhs)) {
            return false;
        }
        if (!isSet(lhs)) {
            return true;
        }
        PtrRepresentation *lhsp = asPtr(lhs);
        PtrRepresentation *rhsp = asPtr(rhs);
        if (lhsp->value != rhsp->value) {
            return false;
        }
        return true;
    } else if (type == RecordTypeId) {
        RecordRepresentation *lhsr = asRecord(lhs);
        RecordRepresentation *rhsr = asRecord(rhs);
        if (lhsr->record_name_length != rhsr->record_name_length) {
            return false;
        }
        if (memcmp(lhsr->record_name, rhsr->record_name, lhsr->record_name_length) != 0) {
            return false;
        }
        assert(lhsr->num_of_fields == rhsr->num_of_fields);
        UnknownBlob **lhsI = getRecordFields(lhsr);
        UnknownBlob **rhsI = getRecordFields(rhsr);
        for (unsigned i = 0; i < lhsr->num_of_fields; ++i, ++lhsI, ++rhsI) {
            if (!_DI_equals(*lhsI, *rhsI)) {
                return false;
            }
        }
        return true;
    } else if (type == FunctionSymbolTypeId) {
        FunctionSymbolRepresentation *lhsf = asFunctionSymbol(lhs);
        FunctionSymbolRepresentation *rhsf = asFunctionSymbol(rhs);
        if (lhsf->symbol_name_length != rhsf->symbol_name_length) {
            return false;
        }
        if (memcmp(lhsf->symbol_name, rhsf->symbol_name, lhsf->symbol_name_length) != 0) {
            return false;
        }
        if (lhsf->num_of_fields != rhsf->num_of_fields) {
            return false;
        }
        UnknownBlob **lhsI = getFunctionSymbolSubterms(lhsf);
        UnknownBlob **rhsI = getFunctionSymbolSubterms(rhsf);
        for (unsigned i = 0; i < lhsf->num_of_fields; ++i, ++lhsI, ++rhsI) {
            if (!_DI_equals(*lhsI, *rhsI)) {
                return false;
            }
        }
        return true;
    } else if (type == VariableTypeId) {
        assert(!isSet(lhs));
        assert(!isSet(rhs));
        return true;
    } else {
        assert(false);
    }
}

extern "C" void _DI_addGoal(bool (***functions)(UnknownBlob *, UnknownBlob *), UnknownBlob *representation)
{
    assert(representation);
    sldGoalList.emplace_back(functions, representation);
    sldLog.push_back(std::make_unique<AddGoal>());
}

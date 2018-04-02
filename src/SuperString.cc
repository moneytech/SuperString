#include "SuperString.hh"

/*-- imports --*/

// std
#include <iostream> // The only thing we need, and just for printing
#include <SuperString.hh>

/*-- definitions --*/

//*-- SuperString
SuperString::SuperString()
        : _sequence(NULL) {
    // nothing go here
}

SuperString::SuperString(const SuperString &other) /*copy*/ {
    this->_sequence = other._sequence;
    this->_sequence->refAdd();
}

SuperString::SuperString(SuperString::StringSequence *sequence)
        : _sequence(sequence) {
    this->_sequence->refAdd();
}

SuperString::~SuperString() {
    if(this->_sequence != NULL && this->_sequence->refRelease() == 0 && this->freeingCost() < this->keepingCost()) {
        delete this->_sequence;
    }
}

SuperString::Size SuperString::length() const {
    if(this->_sequence != NULL) {
        return this->_sequence->length();
    }
    return 0;
}

int SuperString::compareTo(const SuperString &other) const {
    Size thisLength = this->length();
    Size otherLength = other.length();
    Size len = (thisLength < otherLength) ? thisLength : otherLength;
    for(int i = 0; i < len; i++) {
        int thisCodeUnit = this->codeUnitAt(i).ok();
        int otherCodeUnit = other.codeUnitAt(i).ok();
        if(thisCodeUnit < otherCodeUnit) {
            return -1;
        }
        if(thisCodeUnit > otherCodeUnit) {
            return 1;
        }
    }
    if(thisLength < otherLength) return -1;
    if(thisLength > otherLength) return 1;
    return 0;
}

SuperString::Result<int, SuperString::Error> SuperString::codeUnitAt(SuperString::Size index) const {
    if(this->_sequence != NULL) {
        return this->_sequence->codeUnitAt(index);
    }
    return Result<int, SuperString::Error>(Error::Unexpected);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::substring(SuperString::Size startIndex, SuperString::Size endIndex) const {
    if(this->_sequence != NULL) {
        return this->_sequence->substring(startIndex, endIndex);
    }
    return Result<SuperString, Error>(Error::Unexpected);
}

void SuperString::print(std::ostream &stream) const {
    if(this->_sequence != NULL) {
        this->_sequence->print(stream);
    }
}

void SuperString::print(std::ostream &stream, SuperString::Size startIndex, SuperString::Size endIndex) const {
    if(this->_sequence != NULL) {
        this->_sequence->print(stream, startIndex, endIndex);
    }
}

SuperString SuperString::trim() const {
    if(this->_sequence != NULL) {
        return this->_sequence->trim();
    }
    return *this;
}

SuperString SuperString::trimLeft() const {
    if(this->_sequence != NULL) {
        return this->_sequence->trimLeft();
    }
    return *this;
}

SuperString SuperString::trimRight() const {
    if(this->_sequence != NULL) {
        return this->_sequence->trimRight();
    }
    return *this;
}

// TODO: delete this two methods
SuperString::Size SuperString::freeingCost() const {
    return this->_sequence->freeingCost();
}

SuperString::Size SuperString::keepingCost() const {
    return this->_sequence->keepingCost();
}

SuperString SuperString::operator+(const SuperString &other) const {
    ConcatenationSequence *sequence = new ConcatenationSequence(this->_sequence, other._sequence);
    return SuperString(sequence);
}

SuperString SuperString::operator*(Size times) const {
    MultipleSequence *sequence = new MultipleSequence(this->_sequence, times);
    return SuperString(sequence);
}

SuperString &SuperString::operator=(const SuperString &other) {
    if(this != &other) {
        if(this->_sequence->refRelease() == 0) {
            delete this->_sequence;
        }
        this->_sequence = other._sequence;
        this->_sequence->refAdd();
    }
    return *this;
}

SuperString::Bool SuperString::operator==(const SuperString &other) const {
    return this->compareTo(other) == 0;
}

SuperString SuperString::Const(const char *chars, SuperString::Encoding encoding) {
    StringSequence *sequence = NULL;
    switch(encoding) {
        case Encoding::ASCII:
            sequence = new SuperString::ConstASCIISequence((Byte *) chars);
            break;
        case Encoding::UTF8:
            sequence = new SuperString::ConstUTF8Sequence((Byte *) chars);
            break;
        case Encoding::UTF16BE:
            sequence = new SuperString::ConstUTF16BESequence((Byte *) chars);
            break;
        default:
            sequence = new SuperString::ConstUTF32Sequence((Byte *) chars);
            break;
    }
    return SuperString(sequence);
}

SuperString SuperString::Const(const SuperString::Byte *bytes, SuperString::Encoding encoding) {
    return SuperString::Const((char *) bytes, encoding);
}

SuperString SuperString::Copy(const char *chars, Encoding encoding) {
    StringSequence *sequence = NULL;
    switch(encoding) {
        case Encoding::ASCII:
            sequence = new SuperString::CopyASCIISequence((Byte *) chars);
            break;
        case Encoding::UTF8:
            sequence = new SuperString::CopyUTF8Sequence((Byte *) chars);
            break;
        case Encoding::UTF16BE:
            sequence = new SuperString::CopyUTF16BESequence((Byte *) chars);
            break;
        default:
            sequence = new SuperString::CopyUTF32Sequence((Byte *) chars);
            break;
    }
    return SuperString(sequence);
}

SuperString SuperString::Copy(const SuperString::Byte *bytes, Encoding encoding) {
    return SuperString::Copy((char *) bytes, encoding);
}

//*-- SuperString::StringSequence (abstract|internal)
SuperString::StringSequence::StringSequence()
        : _refCount(0) {
    // nothing go here
}

SuperString::StringSequence::~StringSequence() {
    // nothing go here
}

SuperString::Bool SuperString::StringSequence::isEmpty() const {
    return this->length() == 0;
}

SuperString::Bool SuperString::StringSequence::isNotEmpty() const {
    return this->length() > 0;
}

// TODO: make this const
void SuperString::StringSequence::refAdd() {
    this->_refCount++;
}

// TODO: make this const
SuperString::Size SuperString::StringSequence::refRelease() {
    if(this->_refCount == 0) {
        return 0;
    }
    return --this->_refCount;
}

SuperString::Size SuperString::StringSequence::refCount() const {
    return this->_refCount;
}

void SuperString::StringSequence::addReferencer(SuperString::ReferenceStringSequence *sequence) const {
    StringSequence *self = (StringSequence *) (unsigned long) this;
    self->_referencers.push(sequence);
}

void SuperString::StringSequence::removeReferencer(SuperString::ReferenceStringSequence *sequence) const {
    StringSequence *self = (StringSequence *) (unsigned long) this;
    self->_referencers.remove(sequence);
}

SuperString::Size SuperString::StringSequence::freeingCost() const {
    Size cost = 0;
    SingleLinkedList<ReferenceStringSequence *>::Node<ReferenceStringSequence *> *node = this->_referencers._head;
    while(node != NULL) {
        cost += node->_data->reconstructionCost();
        node = node->_next;
    }
    return cost;
}

void SuperString::StringSequence::reconstructReferencers() {
    SingleLinkedList<ReferenceStringSequence *>::Node<ReferenceStringSequence *> *node = this->_referencers._head;
    while(node != NULL) {
        node->_data->reconstruct(this);
        node = node->_next;
    }
}

//*-- SuperString::ReferenceStringSequence (abstract|internal)
SuperString::ReferenceStringSequence::~ReferenceStringSequence() {
    // nothing go here
}

//*-- SuperString::ConstASCIISequence (internal)
SuperString::ConstASCIISequence::ConstASCIISequence(const Byte *bytes)
        : _bytes(bytes),
          _lengthComputed(SuperString::FALSE) {
    // nothing go here
}

SuperString::ConstASCIISequence::~ConstASCIISequence() {
    this->reconstructReferencers();
}

SuperString::Size SuperString::ConstASCIISequence::length() const /*override*/ {
    if(this->_lengthComputed == SuperString::FALSE) {
        ConstASCIISequence *self = ((ConstASCIISequence *) ((Size) this)); // to keep this method `const`
        self->_lengthComputed = SuperString::TRUE;
        self->_length = SuperString::ASCII::length(this->_bytes);
    }
    return this->_length;
}

SuperString::Result<int, SuperString::Error>
SuperString::ConstASCIISequence::codeUnitAt(SuperString::Size index) const {
    if(index < this->length()) {
        return Result<int, SuperString::Error>(SuperString::ASCII::codeUnitAt(this->_bytes, index));
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::ConstASCIISequence::substring(SuperString::Size startIndex,
                                           SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::ConstASCIISequence::print(std::ostream &stream) const {
    SuperString::ASCII::print(stream, this->_bytes);
    return TRUE;
}

SuperString::Bool SuperString::ConstASCIISequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                         SuperString::Size endIndex) const {
    if(this->length() < startIndex || this->length() < endIndex) {
        return FALSE;
    }
    SuperString::ASCII::print(stream, this->_bytes, startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::ConstASCIISequence::trim() const {
    Pair<Size, Size> indexes = SuperString::ASCII::trim(this->_bytes, this->_length);
    return this->substring(indexes.first(), indexes.second()).ok();
}

SuperString SuperString::ConstASCIISequence::trimLeft() const {
    return this->substring(SuperString::ASCII::trimLeft(this->_bytes), this->length()).ok();
}

SuperString SuperString::ConstASCIISequence::trimRight() const {
    return this->substring(0, SuperString::ASCII::trimRight(this->_bytes, this->length())).ok();
}

SuperString::Size SuperString::ConstASCIISequence::keepingCost() const {
    return sizeof(ConstASCIISequence);
}

//*-- SuperString::CopyASCIISequence (internal)
SuperString::CopyASCIISequence::CopyASCIISequence(const SuperString::Byte *bytes) {
    this->_length = SuperString::ASCII::length(bytes);
    this->_data = new Byte[this->_length + 1];
    for(Size i = 0; i <= this->_length; i++) {
        *(this->_data + i) = *(bytes + i);
    }
}

SuperString::CopyASCIISequence::CopyASCIISequence(const SuperString::ConstASCIISequence *sequence) {
    this->_length = sequence->length();
    this->_data = new Byte[this->_length + 1];
    for(Size i = 0; i <= this->_length; i++) {
        *(this->_data + i) = *(sequence->_bytes + i);
    }
}

SuperString::CopyASCIISequence::~CopyASCIISequence() {
    this->reconstructReferencers();
    delete this->_data;
}

SuperString::Size SuperString::CopyASCIISequence::length() const {
    return this->_length;
}

SuperString::Result<int, SuperString::Error> SuperString::CopyASCIISequence::codeUnitAt(
        SuperString::Size index) const {
    if(index < this->length()) {
        return Result<int, SuperString::Error>(SuperString::ASCII::codeUnitAt(this->_data, index));
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::CopyASCIISequence::substring(SuperString::Size startIndex,
                                          SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::CopyASCIISequence::print(std::ostream &stream) const {
    SuperString::ASCII::print(stream, this->_data);
    return TRUE;
}

SuperString::Bool SuperString::CopyASCIISequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                        SuperString::Size endIndex) const {
    if(this->length() < startIndex || this->length() < endIndex) {
        return FALSE;
    }
    SuperString::ASCII::print(stream, this->_data, startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::CopyASCIISequence::trim() const {
    Pair<Size, Size> indexes = SuperString::ASCII::trim(this->_data, this->_length);
    return this->substring(indexes.first(), indexes.second()).ok();
}

SuperString SuperString::CopyASCIISequence::trimLeft() const {
    return this->substring(SuperString::ASCII::trimLeft(this->_data), this->length()).ok();
}

SuperString SuperString::CopyASCIISequence::trimRight() const {
    return this->substring(0, SuperString::ASCII::trimRight(this->_data, this->length())).ok();
}

SuperString::Size SuperString::CopyASCIISequence::keepingCost() const {
    Size cost = sizeof(CopyASCIISequence);
    if(this->_data != NULL) {
        cost += this->length() + 1;
    }
    return cost;
}

//*-- SuperString::ConstUTF8Sequence (internal)
SuperString::ConstUTF8Sequence::ConstUTF8Sequence(const Byte *bytes)
        : _bytes(bytes),
          _lengthComputed(SuperString::FALSE) {
    // nothing go here
}

SuperString::ConstUTF8Sequence::~ConstUTF8Sequence() {
    this->reconstructReferencers();
}

SuperString::Size SuperString::ConstUTF8Sequence::length() const /*override*/ {
    if(this->_lengthComputed == FALSE) {
        ConstUTF8Sequence *self = ((ConstUTF8Sequence *) ((Size) this)); // to keep this method `const`
        self->_lengthComputed = SuperString::TRUE;
        self->_length = SuperString::UTF8::length(this->_bytes);
    }
    return this->_length;
}

SuperString::Result<int, SuperString::Error> SuperString::ConstUTF8Sequence::codeUnitAt(SuperString::Size index) const {
    return SuperString::UTF8::codeUnitAt(this->_bytes, index);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::ConstUTF8Sequence::substring(SuperString::Size startIndex,
                                          SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    Size length = this->length();
    if(length < startIndex || length < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::ConstUTF8Sequence::print(std::ostream &stream) const {
    SuperString::UTF8::print(stream, this->_bytes);
    return TRUE;
}

SuperString::Bool SuperString::ConstUTF8Sequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                        SuperString::Size endIndex) const {
    Size length = this->length();
    if(length < startIndex || length < endIndex) {
        return FALSE;
    }
    SuperString::UTF8::print(stream, this->_bytes, startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::ConstUTF8Sequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::ConstUTF8Sequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::ConstUTF8Sequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::ConstUTF8Sequence::keepingCost() const {
    return sizeof(ConstUTF8Sequence);
}

//*-- SuperString::CopyUTF8Sequence (internal)
SuperString::CopyUTF8Sequence::CopyUTF8Sequence(const SuperString::Byte *bytes) {
    Pair<Size, Size> lengthAndMemoryLength = SuperString::UTF8::lengthAndMemoryLength(bytes);
    this->_length = lengthAndMemoryLength.first();
    this->_memoryLength = lengthAndMemoryLength.second();
    this->_data = new Byte[this->_memoryLength];
    for(Size i = 0; i < this->_memoryLength; i++) {
        *(this->_data + i) = *(bytes + i);
    }
}

SuperString::CopyUTF8Sequence::CopyUTF8Sequence(const SuperString::ConstUTF8Sequence *sequence) {
    Pair<Size, Size> lengthAndMemoryLength = SuperString::UTF8::lengthAndMemoryLength(sequence->_bytes);
    this->_length = lengthAndMemoryLength.first();
    this->_memoryLength = lengthAndMemoryLength.second();
    this->_data = new Byte[this->_memoryLength];
    for(Size i = 0; i < this->_memoryLength; i++) {
        *(this->_data + i) = *(sequence->_bytes + i);
    }
}

SuperString::CopyUTF8Sequence::~CopyUTF8Sequence() {
    this->reconstructReferencers();
    delete this->_data;
}

SuperString::Size SuperString::CopyUTF8Sequence::length() const {
    return this->_length;
}

SuperString::Result<int, SuperString::Error> SuperString::CopyUTF8Sequence::codeUnitAt(SuperString::Size index) const {
    if(index < this->length()) {
        return Result<int, Error>(SuperString::UTF8::codeUnitAt(this->_data, index));
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::CopyUTF8Sequence::substring(SuperString::Size startIndex, SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::CopyUTF8Sequence::print(std::ostream &stream) const {
    SuperString::UTF8::print(stream, this->_data);
    return TRUE;
}

SuperString::Bool SuperString::CopyUTF8Sequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                       SuperString::Size endIndex) const {
    Size length = this->length();
    if(length < startIndex || length < endIndex) {
        return FALSE;
    }
    SuperString::UTF8::print(stream, this->_data, startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::CopyUTF8Sequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::CopyUTF8Sequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::CopyUTF8Sequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::CopyUTF8Sequence::keepingCost() const {
    Size cost = sizeof(CopyUTF8Sequence) + this->_memoryLength;
    return cost;
}

//*-- ConstUTF16BESequence (internal)
SuperString::ConstUTF16BESequence::ConstUTF16BESequence(const SuperString::Byte *bytes)
        : _bytes(bytes),
          _lengthComputed(FALSE) {
    // nothing go here
}

SuperString::ConstUTF16BESequence::~ConstUTF16BESequence() {
    this->reconstructReferencers();
}

SuperString::Size SuperString::ConstUTF16BESequence::length() const /*override*/ {
    if(this->_lengthComputed == SuperString::FALSE) {
        ConstUTF16BESequence *self = ((ConstUTF16BESequence *) ((Size) this)); // to keep this method `const`
        self->_lengthComputed = SuperString::TRUE;
        self->_length = SuperString::UTF16BE::length(this->_bytes);
    }
    return this->_length;
}

SuperString::Result<int, SuperString::Error> SuperString::ConstUTF16BESequence::codeUnitAt(
        SuperString::Size index) const {
    if(index < this->length()) {
        return SuperString::UTF16BE::codeUnitAt(this->_bytes, index);
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::ConstUTF16BESequence::substring(SuperString::Size startIndex,
                                             SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::ConstUTF16BESequence::print(std::ostream &stream) const {
    SuperString::UTF16BE::print(stream, this->_bytes, this->length());
    return TRUE;
}

SuperString::Bool SuperString::ConstUTF16BESequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                           SuperString::Size endIndex) const {
    Size length = this->length();
    if(length < startIndex || length < endIndex) {
        return FALSE;
    }
    SuperString::UTF16BE::print(stream, this->_bytes, startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::ConstUTF16BESequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::ConstUTF16BESequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::ConstUTF16BESequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::ConstUTF16BESequence::keepingCost() const {
    return sizeof(ConstUTF16BESequence);
}

//*-- SuperString::CopyUTF16BESequence (internal)
SuperString::CopyUTF16BESequence::CopyUTF16BESequence(const SuperString::Byte *bytes) {
    Pair<Size, Size> lengthAndMemoryLength = SuperString::UTF16BE::lengthAndMemoryLength(bytes);
    this->_length = lengthAndMemoryLength.first();
    this->_memoryLength = lengthAndMemoryLength.second();
    this->_data = new Byte[this->_memoryLength];
    for(Size i = 0; i < this->_memoryLength; i++) {
        *(this->_data + i) = *(bytes + i);
    }
}

SuperString::CopyUTF16BESequence::CopyUTF16BESequence(const SuperString::ConstUTF16BESequence *sequence) {
    Pair<Size, Size> lengthAndMemoryLength = SuperString::UTF16BE::lengthAndMemoryLength(sequence->_bytes);
    this->_length = lengthAndMemoryLength.first();
    this->_memoryLength = lengthAndMemoryLength.second();
    this->_data = new Byte[this->_memoryLength];
    for(Size i = 0; i < this->_memoryLength; i++) {
        *(this->_data + i) = *(sequence->_bytes + i);
    }
}

SuperString::CopyUTF16BESequence::~CopyUTF16BESequence() {
    this->reconstructReferencers();
    delete this->_data;
}

SuperString::Size SuperString::CopyUTF16BESequence::length() const {
    return this->_length;
}

SuperString::Result<int, SuperString::Error>
SuperString::CopyUTF16BESequence::codeUnitAt(SuperString::Size index) const {
    if(index < this->length()) {
        return SuperString::UTF16BE::codeUnitAt(this->_data, index);
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::CopyUTF16BESequence::substring(SuperString::Size startIndex, SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::CopyUTF16BESequence::print(std::ostream &stream) const {
    SuperString::UTF16BE::print(stream, this->_data, this->length());
    return TRUE;
}

SuperString::Bool SuperString::CopyUTF16BESequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                          SuperString::Size endIndex) const {
    Size length = this->length();
    if(length < startIndex || length < endIndex) {
        return FALSE;
    }
    SuperString::UTF16BE::print(stream, this->_data, startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::CopyUTF16BESequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::CopyUTF16BESequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::CopyUTF16BESequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::CopyUTF16BESequence::keepingCost() const {
    Size cost = sizeof(CopyUTF16BESequence) + this->_memoryLength;
    return cost;
}

//*-- SuperString::ConstUTF32Sequence (internal)
SuperString::ConstUTF32Sequence::ConstUTF32Sequence(const SuperString::Byte *bytes)
        : _bytes(((const int *) bytes)),
          _lengthComputed(SuperString::FALSE) {
    // nothing go here
}

SuperString::ConstUTF32Sequence::~ConstUTF32Sequence() {
    this->reconstructReferencers();
}

SuperString::Size SuperString::ConstUTF32Sequence::length() const /*override*/ {
    if(this->_lengthComputed == SuperString::FALSE) {
        ConstUTF32Sequence *self = ((ConstUTF32Sequence *) ((Size) this)); // to keep this method `const`
        self->_lengthComputed = SuperString::TRUE;
        self->_length = SuperString::UTF32::length(((const Byte *) this->_bytes));
    }
    return this->_length;
}

SuperString::Result<int, SuperString::Error>
SuperString::ConstUTF32Sequence::codeUnitAt(SuperString::Size index) const {
    if(index < this->length()) {
        return Result<int, SuperString::Error>(SuperString::UTF32::codeUnitAt(((const Byte *) this->_bytes), index));
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::ConstUTF32Sequence::substring(SuperString::Size startIndex,
                                           SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::ConstUTF32Sequence::print(std::ostream &stream) const {
    SuperString::UTF32::print(stream, ((const Byte *) this->_bytes));
    return TRUE;
}

SuperString::Bool SuperString::ConstUTF32Sequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                         SuperString::Size endIndex) const {
    if(this->length() < startIndex || this->length() < endIndex) {
        return FALSE;
    }
    SuperString::UTF32::print(stream, ((Byte *) this->_bytes), startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::ConstUTF32Sequence::trim() const {
    Pair<Size, Size> indexes = SuperString::UTF32::trim(((Byte *) this->_bytes), this->_length);
    return this->substring(indexes.first(), indexes.second()).ok();
}

SuperString SuperString::ConstUTF32Sequence::trimLeft() const {
    return this->substring(SuperString::UTF32::trimLeft(((Byte *) this->_bytes)), this->length()).ok();
}

SuperString SuperString::ConstUTF32Sequence::trimRight() const {
    return this->substring(0, SuperString::UTF32::trimRight(((Byte *) this->_bytes), this->length())).ok();
}

SuperString::Size SuperString::ConstUTF32Sequence::keepingCost() const {
    return sizeof(ConstUTF32Sequence);
}

//*-- SuperString::CopyUTF32Sequence (internal)
SuperString::CopyUTF32Sequence::CopyUTF32Sequence(const SuperString::Byte *bytes) {
    this->_length = SuperString::UTF32::length(bytes);
    this->_data = new int[this->_length + 1];
    for(Size i = 0; i <= this->_length; i++) {
        *(this->_data + i) = *(((const int *) bytes) + i);
    }
}

SuperString::CopyUTF32Sequence::CopyUTF32Sequence(const SuperString::ConstUTF32Sequence *sequence) {
    this->_length = SuperString::UTF32::length(((const Byte *) sequence->_bytes));
    this->_data = new int[this->_length + 1];
    for(Size i = 0; i <= this->_length; i++) {
        *(this->_data + i) = *(sequence->_bytes + i);
    }
}

SuperString::CopyUTF32Sequence::~CopyUTF32Sequence() {
    this->reconstructReferencers();
    delete this->_data;
}

SuperString::Size SuperString::CopyUTF32Sequence::length() const {
    return this->_length;
}

SuperString::Result<int, SuperString::Error> SuperString::CopyUTF32Sequence::codeUnitAt(
        SuperString::Size index) const {
    if(index < this->length()) {
        return Result<int, SuperString::Error>(SuperString::UTF32::codeUnitAt(((const Byte *) this->_data), index));
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::CopyUTF32Sequence::substring(SuperString::Size startIndex,
                                          SuperString::Size endIndex) const {
    // TODO: General code, specify + repeated * times
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::CopyUTF32Sequence::print(std::ostream &stream) const {
    SuperString::UTF32::print(stream, ((const Byte *) this->_data));
    return TRUE;
}

SuperString::Bool SuperString::CopyUTF32Sequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                        SuperString::Size endIndex) const {
    if(this->length() < startIndex || this->length() < endIndex) {
        return FALSE;
    }
    SuperString::UTF32::print(stream, ((const Byte *) this->_data), startIndex, endIndex);
    return TRUE;
}

SuperString SuperString::CopyUTF32Sequence::trim() const {
    Pair<Size, Size> indexes = SuperString::UTF32::trim(((const Byte *) this->_data), this->_length);
    return this->substring(indexes.first(), indexes.second()).ok();
}

SuperString SuperString::CopyUTF32Sequence::trimLeft() const {
    return this->substring(SuperString::UTF32::trimLeft(((const Byte *) this->_data)), this->length()).ok();
}

SuperString SuperString::CopyUTF32Sequence::trimRight() const {
    return this->substring(0, SuperString::UTF32::trimRight(((const Byte *) this->_data), this->length())).ok();
}

SuperString::Size SuperString::CopyUTF32Sequence::keepingCost() const {
    Size cost = sizeof(CopyASCIISequence);
    if(this->_data != NULL) {
        cost += this->length() + 1;
    }
    return cost;
}

//*-- SuperString::SubstringSequence (internal)
SuperString::SubstringSequence::SubstringSequence(const StringSequence *sequence, SuperString::Size startIndex,
                                                  SuperString::Size endIndex) {
    this->_kind = Kind::SUBSTRING;
    this->_container._substring._sequence = sequence;
    this->_container._substring._startIndex = startIndex;
    this->_container._substring._endIndex = endIndex;
    this->_container._substring._sequence->addReferencer(this);
}

SuperString::SubstringSequence::~SubstringSequence() {
    switch(this->_kind) {
        case Kind::SUBSTRING:
            this->_container._substring._sequence->removeReferencer(this);
            if(this->_container._substring._sequence->refCount() == 0 && this->freeingCost() < this->keepingCost()) {
                delete this->_container._substring._sequence;
            }
            break;
        case Kind::CONTENTED:
            delete this->_container._contented._data;
            break;
    }
}

SuperString::Size SuperString::SubstringSequence::length() const /*override*/ {
    switch(this->_kind) {
        case Kind::SUBSTRING:
            return this->_container._substring._endIndex - this->_container._substring._startIndex;
        case Kind::CONTENTED:
            return this->_container._contented._length;
    }
}

SuperString::Result<int, SuperString::Error> SuperString::SubstringSequence::codeUnitAt(
        SuperString::Size index) const {
    if(index < this->length()) {
        switch(this->_kind) {
            case Kind::SUBSTRING:
                return this->_container._substring._sequence->codeUnitAt(
                        this->_container._substring._startIndex + index);
            case Kind::CONTENTED:
                return Result<int, SuperString::Error>(*(this->_container._contented._data + index));
        }
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::SubstringSequence::substring(SuperString::Size startIndex, SuperString::Size endIndex) const {
    if(this->_kind == Kind::SUBSTRING) {
        if((this->_container._substring._sequence->length() < (this->_container._substring._startIndex + startIndex)) ||
           (this->_container._substring._sequence->length() < (this->_container._substring._startIndex + endIndex))) {
            return Result<SuperString, Error>(Error::RangeError);
        } else {
            return Result<SuperString, Error>(SuperString(new SubstringSequence(this->_container._substring._sequence,
                                                                                this->_container._substring._startIndex +
                                                                                startIndex,
                                                                                this->_container._substring._startIndex +
                                                                                endIndex)));
        }
    }
    // TODO:
    return Result<SuperString, Error>(Error::Unimplemented);
}

SuperString::Bool SuperString::SubstringSequence::print(std::ostream &stream) const {
    switch(this->_kind) {
        case Kind::SUBSTRING:
            return this->_container._substring._sequence->print(stream, this->_container._substring._startIndex,
                                                                this->_container._substring._endIndex);
        case Kind::CONTENTED:
            SuperString::UTF32::print(stream, ((Byte *) this->_container._contented._data));
            return TRUE;
    }
}

SuperString::Bool SuperString::SubstringSequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                        SuperString::Size endIndex) const {
    switch(this->_kind) {
        case Kind::SUBSTRING:
            return this->_container._substring._sequence->print(stream,
                                                                this->_container._substring._startIndex + startIndex,
                                                                this->_container._substring._startIndex + endIndex);
        case Kind::CONTENTED:
            SuperString::UTF32::print(stream, ((Byte *) this->_container._contented._data), startIndex, endIndex);
            return TRUE;
    }
}

SuperString SuperString::SubstringSequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::SubstringSequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::SubstringSequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::SubstringSequence::keepingCost() const {
    if(this->_kind == Kind::SUBSTRING) {
        return sizeof(SubstringSequence) + this->_container._substring._sequence->keepingCost();
    } else {
        return sizeof(SubstringSequence) + this->_container._contented._length * sizeof(int);
    }
}

SuperString::Size SuperString::SubstringSequence::reconstructionCost() const {
    if(this->_kind == Kind::SUBSTRING) {
        return sizeof(SubstringSequence) +
               (this->_container._substring._endIndex - this->_container._substring._startIndex) * sizeof(int);
    }
    return 0;
}

void SuperString::SubstringSequence::reconstruct(const StringSequence *sequence) const {
    struct SubstringMetaInfo old = this->_container._substring;
    SubstringSequence *self = ((SubstringSequence *) ((Size) this));
    self->_kind = Kind::CONTENTED;
    self->_container._contented._length = old._endIndex - old._startIndex;
    self->_container._contented._data = new int[self->_container._contented._length];
    for(Size i = 0; i < self->_container._contented._length; i++) {
        self->_container._contented._data[i] = old._sequence->codeUnitAt(old._startIndex + i).ok();
    }
}

//*-- SuperString::ConcatenationSequence (internal)
SuperString::ConcatenationSequence::ConcatenationSequence(const StringSequence *leftSequence,
                                                          const StringSequence *rightSequence) {
    this->_kind = Kind::CONCATENATION;
    this->_container._concatenation._left = leftSequence;
    this->_container._concatenation._right = rightSequence;
    this->_container._concatenation._left->addReferencer(this);
    this->_container._concatenation._right->addReferencer(this);
}

SuperString::ConcatenationSequence::~ConcatenationSequence() {
    this->reconstructReferencers();
    if(this->_kind == Kind::CONCATENATION) {
        this->_container._concatenation._left->removeReferencer(this);
        this->_container._concatenation._right->removeReferencer(this);
        if(this->_container._concatenation._left->refCount() == 0 && this->freeingCost() < this->keepingCost()) {
            delete this->_container._concatenation._left;
        }
        if(this->_container._concatenation._right->refCount() == 0 && this->freeingCost() < this->keepingCost()) {
            delete this->_container._concatenation._right;
        }
    }
}

SuperString::Size SuperString::ConcatenationSequence::length() const {
    if(this->_kind == Kind::CONCATENATION) {
        return this->_container._concatenation._left->length() + this->_container._concatenation._right->length();
    }
    // TODO:
    return 0;
}

SuperString::Result<int, SuperString::Error>
SuperString::ConcatenationSequence::codeUnitAt(SuperString::Size index) const {
    if(this->_kind == Kind::CONCATENATION) {
        if(index < this->_container._concatenation._left->length()) {
            return this->_container._concatenation._left->codeUnitAt(index);
        } else if((index - this->_container._concatenation._left->length()) <
                  this->_container._concatenation._right->length()) {
            return this->_container._concatenation._right->codeUnitAt(
                    index - this->_container._concatenation._left->length());
        } else {
            return Result<int, Error>(Error::RangeError);
        }
    }
    // TODO:
    return Result<int, Error>(Error::Unimplemented);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::ConcatenationSequence::substring(SuperString::Size startIndex,
                                              SuperString::Size endIndex) const {
    if(this->length() < startIndex || this->length() < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::ConcatenationSequence::print(std::ostream &stream) const {
    if(this->_kind == Kind::CONCATENATION) {
        this->_container._concatenation._left->print(stream);
        this->_container._concatenation._right->print(stream);
    }
    return FALSE; // TODO
}

SuperString::Bool SuperString::ConcatenationSequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                            SuperString::Size endIndex) const {
    if(this->_kind == Kind::CONCATENATION) {
        if(startIndex < this->_container._concatenation._left->length()) {
            if(endIndex < this->_container._concatenation._left->length()) {
                this->_container._concatenation._left->print(stream, startIndex, endIndex);
            } else {
                this->_container._concatenation._left->print(stream, startIndex,
                                                             this->_container._concatenation._left->length());
                this->_container._concatenation._right->print(stream, 0,
                                                              endIndex -
                                                              this->_container._concatenation._left->length());
            }
        } else {
            if((endIndex - this->_container._concatenation._left->length()) <
               this->_container._concatenation._right->length()) {
                this->_container._concatenation._right->print(stream, startIndex -
                                                                      this->_container._concatenation._left->length(),
                                                              endIndex -
                                                              this->_container._concatenation._left->length());
            }
        }
    }
    return FALSE; // TODO:
}

SuperString SuperString::ConcatenationSequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::ConcatenationSequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::ConcatenationSequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::ConcatenationSequence::keepingCost() const {
    if(this->_kind == Kind::CONCATENATION) {
        return sizeof(SubstringSequence) + this->_container._concatenation._left->keepingCost() +
               this->_container._concatenation._right->keepingCost();
    }
    return 0;
}

SuperString::Size SuperString::ConcatenationSequence::reconstructionCost() const {
    if(this->_kind == Kind::CONCATENATION) {
        return sizeof(SubstringSequence) +
               (this->_container._concatenation._left->length() + this->_container._concatenation._right->length()) * 2;
    }
    return 0;
}

void SuperString::ConcatenationSequence::reconstruct(const StringSequence *sequence) const {

}

//*-- MultipleSequence (internal)
SuperString::MultipleSequence::MultipleSequence(const StringSequence *sequence, Size time) {
    this->_kind = Kind::MULTIPLE;
    this->_container._multiple._time = time;
    this->_container._multiple._sequence = sequence;
    this->_container._multiple._sequence->addReferencer(this);
}

SuperString::MultipleSequence::~MultipleSequence() {
    this->reconstructReferencers();
    if(this->_kind == Kind::MULTIPLE) {
        this->_container._multiple._sequence->removeReferencer(this);
        if(this->_container._multiple._sequence->refCount() == 0 && this->freeingCost() < this->keepingCost()) {
            delete this->_container._multiple._sequence;
        }
    }
}

SuperString::Size SuperString::MultipleSequence::length() const {
    if(this->_kind == Kind::MULTIPLE) {
        return this->_container._multiple._sequence->length() * this->_container._multiple._time;
    }
    // TODO:
    return 0;
}

SuperString::Result<int, SuperString::Error> SuperString::MultipleSequence::codeUnitAt(SuperString::Size index) const {
    if(this->_kind == Kind::MULTIPLE) {
        Size length = this->length();
        if(index < length) {
            return this->_container._multiple._sequence->codeUnitAt(
                    index % this->_container._multiple._sequence->length());
        }
        return Result<int, Error>(Error::RangeError);
    }
    // TODO:
    return Result<int, Error>(Error::Unimplemented);
}

SuperString::Result<SuperString, SuperString::Error>
SuperString::MultipleSequence::substring(SuperString::Size startIndex,
                                         SuperString::Size endIndex) const {
    Size length = this->length();
    if(length < startIndex || length < endIndex) {
        return Result<SuperString, Error>(Error::RangeError);
    }
    SubstringSequence *sequence = new SubstringSequence(this, startIndex, endIndex);
    return Result<SuperString, Error>(SuperString(sequence));
}

SuperString::Bool SuperString::MultipleSequence::print(std::ostream &stream) const {
    if(this->_kind == Kind::MULTIPLE) {
        for(Size i = 0; i < this->_container._multiple._time; i++) {
            this->_container._multiple._sequence->print(stream);
        }
    }
    return FALSE; // TODO
}

SuperString::Bool SuperString::MultipleSequence::print(std::ostream &stream, SuperString::Size startIndex,
                                                       SuperString::Size endIndex) const {
    if(this->_kind == Kind::MULTIPLE) {
        Bool printing = FALSE;
        Size unitLength = this->_container._multiple._sequence->length();
        for(Size i = 0; i < this->_container._multiple._time; i++) {
            Size iterationStartIndex = i * unitLength;
            Size iterationEndIndex = (i + 1) * unitLength;
            if(printing == FALSE) {
                if(iterationStartIndex <= startIndex) {
                    if(endIndex < iterationEndIndex) {
                        this->_container._multiple._sequence->print(stream, startIndex - iterationStartIndex,
                                                                    endIndex - iterationStartIndex);
                        break;
                    } else {
                        printing = TRUE;
                        this->_container._multiple._sequence->print(stream, startIndex - iterationStartIndex,
                                                                    unitLength);
                    }
                }
            } else {
                if(endIndex <= iterationEndIndex) {
                    this->_container._multiple._sequence->print(stream, 0, endIndex - iterationStartIndex);
                }
            }
        }
    }
    return FALSE; // TODO
}

SuperString SuperString::MultipleSequence::trim() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(startIndex, endIndex).ok(); // TODO:
}

SuperString SuperString::MultipleSequence::trimLeft() const {
    // TODO: General code, specify
    Size startIndex = 0;
    Result<int, Error> result = this->codeUnitAt(startIndex);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(++startIndex);
    }
    return this->substring(startIndex, this->length()).ok(); // TODO:
}

SuperString SuperString::MultipleSequence::trimRight() const {
    // TODO: General code, specify
    Size endIndex = this->length();
    Result<int, Error> result = this->codeUnitAt(endIndex - 1);
    while(result.isOk() && SuperString::isWhiteSpace(result.ok())) {
        result = this->codeUnitAt(--endIndex - 1);
    }
    return this->substring(0, endIndex).ok(); // TODO:
}

SuperString::Size SuperString::MultipleSequence::keepingCost() const {
    if(this->_kind == Kind::MULTIPLE) {
        return sizeof(SubstringSequence) + this->_container._multiple._sequence->keepingCost();
    }
    return 0;
}

SuperString::Size SuperString::MultipleSequence::reconstructionCost() const {
    if(this->_kind == Kind::MULTIPLE) {
        return sizeof(SubstringSequence) +
               this->_container._multiple._sequence->length() * this->_container._multiple._time * 2;
    }
    return 0;
}

void SuperString::MultipleSequence::reconstruct(const StringSequence *sequence) const {

}

//*-- SuperString::ASCII
SuperString::Size SuperString::ASCII::length(const SuperString::Byte *bytes) {
    const Byte *pointer = bytes;
    while(*pointer != 0x00) {
        pointer++;
    }
    return pointer - bytes;
}

int SuperString::ASCII::codeUnitAt(const SuperString::Byte *bytes, SuperString::Size index) {
    return ((int) *(bytes + index));
}

void SuperString::ASCII::print(std::ostream &stream, const SuperString::Byte *bytes) {
    stream << (const char *) bytes;
}

void SuperString::ASCII::print(std::ostream &stream, const SuperString::Byte *bytes, SuperString::Size startIndex,
                               SuperString::Size endIndex) {
    stream.write(((char *) (bytes + startIndex)), endIndex - startIndex);
}

SuperString::Pair<SuperString::Size, SuperString::Size>
SuperString::ASCII::trim(const SuperString::Byte *bytes, SuperString::Size length) {
    Size startIndex = 0;
    Size endIndex = length;
    char c = *(bytes + startIndex);
    while(c != 0x00 && SuperString::isWhiteSpace(c)) {
        c = *(bytes + (++startIndex));
    }
    c = *(bytes + (endIndex - 1));
    while(endIndex > 0 && SuperString::isWhiteSpace(c)) {
        c = *(bytes + (--endIndex - 1));
    }
    return Pair<Size, Size>(startIndex, endIndex);
}

SuperString::Size SuperString::ASCII::trimLeft(const SuperString::Byte *bytes) {
    Size startIndex = 0;
    char c = *(bytes + startIndex);
    while(c != 0x00 && SuperString::isWhiteSpace(c)) {
        c = *(bytes + ++startIndex);
    }
    return startIndex;
}

SuperString::Size SuperString::ASCII::trimRight(const SuperString::Byte *bytes, SuperString::Size length) {
    Size endIndex = length;
    char c = *(bytes + (endIndex - 1));
    while(endIndex > 0 && SuperString::isWhiteSpace(c)) {
        c = *(bytes + (--endIndex - 1));
    }
    return endIndex;
}

// SuperString::UTF8
SuperString::Size SuperString::UTF8::length(const SuperString::Byte *bytes) {
    Size length = 0;
    const Byte *pointer = bytes;
    while(*pointer != 0x00) {
        if((*pointer & 0xf8) == 0xf0) { pointer += 4; }
        else if((*pointer & 0xf0) == 0xe0) { pointer += 3; }
        else if((*pointer & 0xe0) == 0xc0) { pointer += 2; }
        else if((*pointer & 0x80) == 0x00) { pointer++; }
        else return 0;
        length++;
    }
    return length;
}

SuperString::Pair<SuperString::Size, SuperString::Size>
SuperString::UTF8::lengthAndMemoryLength(const SuperString::Byte *bytes) {
    Size length = 0;
    const Byte *pointer = bytes;
    while(*pointer != 0x00) {
        if((*pointer & 0xf8) == 0xf0) { pointer += 4; }
        else if((*pointer & 0xf0) == 0xe0) { pointer += 3; }
        else if((*pointer & 0xe0) == 0xc0) { pointer += 2; }
        else if((*pointer & 0x80) == 0x00) { pointer++; }
        else return Pair<Size, Size>(0, 0); // handle error
        length++;
    }
    return Pair<Size, Size>(length, pointer - bytes + 1);
}

SuperString::Result<int, SuperString::Error>
SuperString::UTF8::codeUnitAt(const SuperString::Byte *bytes, SuperString::Size index) {
    Size i = 0;
    const Byte *pointer = bytes;
    while(*pointer != 0x00) {
        int codeUnit = 0;
        int remainingBytes = 0;
        if((*pointer & 0xf8) == 0xf0) {
            codeUnit = *pointer & 0x07;
            remainingBytes = 3;
        } else if((*pointer & 0xf0) == 0xe0) {
            codeUnit = *pointer & 0x0f;
            remainingBytes = 2;
        } else if((*pointer & 0xe0) == 0xc0) {
            codeUnit = *pointer & 0x1f;
            remainingBytes = 1;
        } else if((*pointer & 0x80) == 0x00) {
            codeUnit = *pointer;
        } else {
            return Result<int, SuperString::Error>(Error::InvalidByteSequence);
        }
        while(remainingBytes-- > 0) {
            pointer++;
            codeUnit = codeUnit << 6 | (*pointer & 0x3f);
        }
        if(i == index) {
            return Result<int, SuperString::Error>(codeUnit);
        }
        pointer++;
        i++;
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

void SuperString::UTF8::print(std::ostream &stream, const SuperString::Byte *bytes) {
    stream << bytes;
}

void SuperString::UTF8::print(std::ostream &stream, const SuperString::Byte *bytes, SuperString::Size startIndex,
                              SuperString::Size endIndex) {
    Result<Pair<Size, Size>, Error> result = SuperString::UTF8::rangeIndexes(bytes, startIndex, endIndex);
    if(result.isOk()) {
        stream.write(((char *) (bytes + result.ok().first())), result.ok().second() - result.ok().first());
    }
}

SuperString::Result<SuperString::Pair<SuperString::Size, SuperString::Size>, SuperString::Error>
SuperString::UTF8::rangeIndexes(
        const SuperString::Byte *bytes, SuperString::Size startIndex, SuperString::Size endIndex) {
    Size i = 0;
    Size startOffset, endOffset;
    Bool first = FALSE, second = FALSE;
    const Byte *pointer = bytes;
    while(*pointer != '\0') {
        if(!first) {
            if(i == startIndex) {
                first = TRUE;
                startOffset = ((Size) pointer) - ((Size) bytes);
            }
        } else {
            if(i == endIndex) {
                second = TRUE;
                endOffset = ((Size) pointer) - ((Size) bytes);
                break;
            }
        }
        if((*pointer & 0xf8) == 0xf0) { pointer += 4; }
        else if((*pointer & 0xf0) == 0xe0) { pointer += 3; }
        else if((*pointer & 0xe0) == 0xc0) { pointer += 2; }
        else if((*pointer & 0x80) == 0x00) { pointer++; }
        else return Result<Pair<Size, Size>, Error>(Error::InvalidByteSequence);
        i++;
    }
    if(!second) {
        if(i == endIndex) {
            second = TRUE;
            endOffset = ((Size) pointer) - ((Size) bytes);
        }
    }
    if(first && second) {
        return Result<Pair<Size, Size>, Error>(Pair<Size, Size>(startOffset, endOffset));
    }
    return Result<Pair<Size, Size>, Error>(Error::RangeError);
}

SuperString::Pair<SuperString::Byte *, SuperString::Size> SuperString::UTF8::codeUnitToChar(int c) {
    char numBytes = 0;
    Byte byte1, byte2, byte3, byte4;
    if(c < 0x0080) {
        byte1 = (Byte) c;
        numBytes = 1;
    } else if(c < 0x0800) {
        byte1 = (Byte) ((c >> 6) + (((Byte) 0b110) << 5));
        byte2 = (Byte) ((((Byte) 0x2) << 6) + (c & 0b00000111111));
        numBytes = 2;
    } else if(c < 0x10000) {
        byte1 = (Byte) ((c >> 12) + (((Byte) 0b1110) << 4));
        byte2 = (Byte) ((((Byte) 0x2) << 6) + (c >> 6 & 0b0000111111));
        byte3 = (Byte) ((((Byte) 0x2) << 6) + (c & 0b0000000000111111));
        numBytes = 3;
    } else if(c < 0x200000) {
        byte1 = (Byte) ((c >> 18) + (((Byte) 0b11110) << 3));
        byte2 = (Byte) ((((Byte) 0x2) << 6) + (c >> 12 & 0b000111111));
        byte3 = (Byte) ((((Byte) 0x2) << 6) + (c >> 6 & 0b000000000111111));
        byte4 = (Byte) ((((Byte) 0x2) << 6) + (c & 0b00000000000000111111));
        numBytes = 4;
    }
    Byte *bytes = new Byte[numBytes];
    Size j = 0;
    while(j < numBytes) {
        switch(j) {
            case 0:
                bytes[j] = byte1;
                break;
            case 1:
                bytes[j] = byte2;
                break;
            case 2:
                bytes[j] = byte3;
                break;
            case 3:
                bytes[j] = byte4;
                break;
            default:;
        }
        j++;
    }
    return Pair<Byte *, Size>(bytes, numBytes);
}

// SuperString::UTF16BE
SuperString::Size SuperString::UTF16BE::length(const SuperString::Byte *bytes) {
    const Byte *pointer = bytes;
    while(*pointer != 0x00 || *(pointer + 1) != 0x00) {
        pointer += 2;
    }
    return (pointer - bytes) / 2;
}

SuperString::Pair<SuperString::Size, SuperString::Size>
SuperString::UTF16BE::lengthAndMemoryLength(const SuperString::Byte *bytes) {
    Size length = 0;
    const Byte *pointer = bytes;
    while(*pointer != 0x00 || *(pointer + 1) != 0x00) {
        if((*pointer & 0xfc) == 0xd8) { pointer += 4; }
        else { pointer += 2; }
        length++;
    }
    return Pair<Size, Size>(length, pointer - bytes + 2);
}

SuperString::Result<int, SuperString::Error>
SuperString::UTF16BE::codeUnitAt(const SuperString::Byte *bytes, SuperString::Size index) {
    Size i = 0;
    const Byte *pointer = bytes;
    while(*pointer != 0x00 || *(pointer + 1) != 0x00) {
        int codeUnit = 0;
        if((*pointer & 0xfc) == 0xd8) {
            codeUnit = (*pointer & 0x03) << 18;
            codeUnit += *(pointer + 1) << 10;
            codeUnit += (*(pointer + 2) & 0x03) << 8;
            codeUnit += *(pointer + 3);
            pointer += 4;
        } else {
            codeUnit = (*pointer << 8) + (*(pointer + 1));
            pointer += 2;
        }
        if(i == index) {
            return Result<int, SuperString::Error>(codeUnit);
        }
        i++;
    }
    return Result<int, SuperString::Error>(Error::RangeError);
}

void SuperString::UTF16BE::print(std::ostream &stream, const SuperString::Byte *bytes, SuperString::Size length) {
    SuperString::UTF16BE::print(stream, bytes, 0, length);
}

void SuperString::UTF16BE::print(std::ostream &stream, const SuperString::Byte *bytes, SuperString::Size startIndex,
                                 SuperString::Size endIndex) {
    Size i = 0;
    const Byte *pointer = bytes;
    while((*pointer != 0x00 || *(pointer + 1) != 0x00) && i < endIndex) {
        int codeUnit = 0;
        if((*pointer & 0xfc) == 0xd8) {
            codeUnit = (*pointer & 0x03) << 18;
            codeUnit += *(pointer + 1) << 10;
            codeUnit += (*(pointer + 2) & 0x03) << 8;
            codeUnit += *(pointer + 3);
            pointer += 4;
        } else {
            codeUnit = (*pointer << 8) + (*(pointer + 1));
            pointer += 2;
        }
        if(startIndex <= i) {
            Pair<Byte *, Size> encoded = SuperString::UTF8::codeUnitToChar(codeUnit);
            stream.write((const char *) encoded.first(), encoded.second());
            delete encoded.first();
        }
        i++;
    }
}

// SuperString::UTF32
SuperString::Size SuperString::UTF32::length(const SuperString::Byte *bytes) {
    const Byte *pointer = bytes;
    while(*((int *) pointer) != 0x00) {
        pointer += 4;
    }
    return (pointer - bytes) / 4;
}

SuperString::Pair<SuperString::Size, SuperString::Size> SuperString::UTF32::lengthAndMemoryLength(
        const SuperString::Byte *bytes) {
    Size length = 0;
    const Byte *pointer = bytes;
    while(*((int *) pointer) != 0x00) {
        pointer += 4;
        length++;
    }
    return Pair<Size, Size>(length, pointer - bytes + 4);
}

int SuperString::UTF32::codeUnitAt(const SuperString::Byte *bytes, SuperString::Size index) {
    return *(((int *) bytes) + index);
}

void SuperString::UTF32::print(std::ostream &stream, const SuperString::Byte *bytes) {
    const Byte *pointer = bytes;
    while(*((int *) pointer) != 0x00) {
        int codeUnit = *((int *) pointer);
        Pair<Byte *, Size> encoded = SuperString::UTF8::codeUnitToChar(codeUnit);
        stream.write((const char *) encoded.first(), encoded.second());
        delete encoded.first();
        pointer += 4;
    }
}

void SuperString::UTF32::print(std::ostream &stream, const SuperString::Byte *bytes, SuperString::Size startIndex,
                               SuperString::Size endIndex) {
    for(Size i = startIndex; i < endIndex; i++) {
        int codeUnit = *(((int *) bytes) + i);
        Pair<Byte *, Size> encoded = SuperString::UTF8::codeUnitToChar(codeUnit);
        stream.write((const char *) encoded.first(), encoded.second());
        delete encoded.first();
    }
}

SuperString::Pair<SuperString::Size, SuperString::Size>
SuperString::UTF32::trim(const SuperString::Byte *bytes, SuperString::Size length) {
    Size startIndex = 0;
    Size endIndex = length;
    int c = *(((const int *) bytes) + startIndex);
    while(c != 0x00 && SuperString::isWhiteSpace(c)) {
        c = *(((const int *) bytes) + ++startIndex);
    }
    c = *(((const int *) bytes) + (endIndex - 1));
    while(endIndex > 0 && SuperString::isWhiteSpace(c)) {
        c = *(((const int *) bytes) + (--endIndex - 1));
    }
    return Pair<Size, Size>(startIndex, endIndex);
}

SuperString::Size SuperString::UTF32::trimLeft(const SuperString::Byte *bytes) {
    Size startIndex = 0;
    int c = *(((const int *) bytes) + startIndex);
    while(c != 0x00 && SuperString::isWhiteSpace(c)) {
        c = *(((const int *) bytes) + ++startIndex);
    }
    return startIndex;
}

SuperString::Size SuperString::UTF32::trimRight(const SuperString::Byte *bytes, SuperString::Size length) {
    Size endIndex = length;
    int c = *(((const int *) bytes) + (endIndex - 1));
    while(endIndex > 0 && SuperString::isWhiteSpace(c)) {
        c = *(((const int *) bytes) + (--endIndex - 1));
    }
    return endIndex;
}

//
std::ostream &operator<<(std::ostream &stream, const SuperString &string) {
    string.print(stream);
    return stream;
}

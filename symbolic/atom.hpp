

#ifndef __ATOM_H__
#define __ATOM_H__

template <typename T> class Atom : public Node {
public:
  Atom() : Node() { val_ = (T)0; }
  Atom(T v) : Node() { val_ = v; }
  vector<shared_ptr<Node> > getValues() {
    vector<shared_ptr<Node> > vec;
    shared_ptr<Node> v(this);
    vec.push_back(v);
    return vec;
  }
  bool isAtom() const { return true; }
  T getConstant() const { return val_; }

private:
  T val_;
};

class Boolean : public Atom<bool> {
public:
  Boolean() : Atom<bool>() {}
  Boolean(bool v) : Atom<bool>(v) {}
  string getHead() { return head_; }
  void toCCode(ostringstream &o) { o << (getConstant() ? "true" : "false"); }
  void toString(ostringstream &o) { toCCode(o); }
  string toCCode() {
    ostringstream o;
    toCCode(o);
    return o.str();
  }
  string toString() {
    ostringstream o;
    toString(o);
    return o.str();
  }

private:
  string head_ = "Boolean";
};

class Integer : public Atom<int64_t> {
public:
  Integer() : Atom<int64_t>() {}
  Integer(int64_t v) : Atom<int64_t>(v) {}
  string getHead() { return head_; }
  void toCCode(ostringstream &o) { o << getConstant(); }
  void toString(ostringstream &o) { toCCode(o); }
  string toCCode() {
    ostringstream o;
    toCCode(o);
    return o.str();
  }
  string toString() {
    ostringstream o;
    toString(o);
    return o.str();
  }

private:
  string head_ = "Integer";
};

class Real : public Atom<double> {
public:
  Real() : Atom<double>() {}
  Real(double v) : Atom<double>(v) {}
  string getHead() { return head_; }
  void toCCode(ostringstream &o) { o << getConstant(); }
  void toString(ostringstream &o) { toCCode(o); }
  string toCCode() {
    ostringstream o;
    toCCode(o);
    return o.str();
  }
  string toString() {
    ostringstream o;
    toString(o);
    return o.str();
  }

private:
  string head_ = "Real";
};

class String : public Atom<string> {
public:
  String() : Atom<string>() {}
  String(string v) : Atom<string>(v) {}
  String(const char *v) : Atom<string>(string(v)) {}
  string getHead() { return head_; }
  void toCCode(ostringstream &o) { o << getConstant(); }
  void toString(ostringstream &o) { toCCode(o); }
  string toCCode() {
    ostringstream o;
    toCCode(o);
    return o.str();
  }
  string toString() {
    ostringstream o;
    toString(o);
    return o.str();
  }

private:
  string head_ = "String";
};
#endif /* __ATOM_H__ */
//----------------------------------------------------------------------
// FILE: vm.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Carolyn Bozin
// DESC: VM implementation file
//----------------------------------------------------------------------

#include <iostream>
#include "vm.h"
#include "mypl_exception.h"


using namespace std;


void VM::error(string msg) const
{
  throw MyPLException::VMError(msg);
}

void VM::error(string msg, const VMFrame& frame) const
{
  int pc = frame.pc - 1;
  VMInstr instr = frame.info.instructions[pc];
  string name = frame.info.function_name;
  msg += " (in " + name + " at " + to_string(pc) + ": " +
    to_string(instr) + ")";
  throw MyPLException::VMError(msg);
}


string to_string(const VM& vm)
{
  string s = "";
  for (const auto& entry : vm.frame_info) {
    const string& name = entry.first;
    s += "\nFrame '" + name + "'\n";
    const VMFrameInfo& frame = entry.second;
    for (int i = 0; i < frame.instructions.size(); ++i) {
      VMInstr instr = frame.instructions[i];
      s += "  " + to_string(i) + ": " + to_string(instr) + "\n"; 
    }
  }
  return s;
}


void VM::add(const VMFrameInfo& frame)
{
  frame_info[frame.function_name] = frame;
}

void VM::run(bool DEBUG)
{
  // grab the "main" frame if it exists
  if (!frame_info.contains("main"))
    error("No 'main' function");
  shared_ptr<VMFrame> frame = make_shared<VMFrame>();
  frame->info = frame_info["main"];
  call_stack.push(frame);

  // run loop (keep going until we run out of instructions)
  while (!call_stack.empty() and frame->pc < frame->info.instructions.size()) {

    // get the next instruction
    VMInstr& instr = frame->info.instructions[frame->pc];

    // increment the program counter
    ++frame->pc;

    // for debugging
    if (DEBUG) {
      cerr << endl << endl;
      cerr << "\t FRAME.........: " << frame->info.function_name << endl;
      cerr << "\t PC............: " << (frame->pc - 1) << endl;
      cerr << "\t INSTR.........: " << to_string(instr) << endl;
      cerr << "\t NEXT OPERAND..: ";
      if (!frame->operand_stack.empty())
        cerr << to_string(frame->operand_stack.top()) << endl;
      else
        cerr << "empty" << endl;
      cerr << "\t NEXT FUNCTION.: ";
      if (!call_stack.empty())
        cerr << call_stack.top()->info.function_name << endl;
      else
        cerr << "empty" << endl;
    }

    //----------------------------------------------------------------------
    // Literals and Variables
    //----------------------------------------------------------------------

    if (instr.opcode() == OpCode::PUSH) {
      frame->operand_stack.push(instr.operand().value());
    }

    else if (instr.opcode() == OpCode::POP) {
      frame->operand_stack.pop();
    }

    else if(instr.opcode() == OpCode::LOAD){
    
      VMValue v = instr.operand().value();
      ensure_not_null(*frame, v);
      //check for int
      if(std::holds_alternative<int>(v)){
        int i = std::get<int>(v);

        //get value from location i and push
        VMValue val = frame->variables[i];
        frame->operand_stack.push(val);

      }else{
        error("Non int index in LOAD instr", *frame);
      }      
    }

    else if(instr.opcode() == OpCode::STORE){

      VMValue v = instr.operand().value();
      ensure_not_null(*frame, v);
      //check for int
      if(std::holds_alternative<int>(v)){
        int i = std::get<int>(v);

        //check if i is past variables sz
        if(i == frame->variables.size()){
          frame->variables.push_back(frame->operand_stack.top());
        }else{
          //store top of stack into memory
          frame->variables[i] = frame->operand_stack.top();
        }
        frame->operand_stack.pop();

      }else{
        error("Non int index in STORE instr", *frame);
      }      
    }

    //----------------------------------------------------------------------
    // Operations
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ADD) {
      //pop x & y, push x + y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(add(y, x));
    }

    else if(instr.opcode() == OpCode::SUB){
      //pop x & y, push x - y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(sub(y, x));
    }

    else if(instr.opcode() == OpCode::MUL){
      //pop x & y, push x * y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(mul(y, x));
    }

    else if(instr.opcode() == OpCode::DIV){
        //pop x & y, push x / y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(div(y, x));
    }

    else if(instr.opcode() == OpCode::AND){
        //pop x & y, push x and y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<bool>(x) and get<bool>(y));
    }

    else if(instr.opcode() == OpCode::OR){
        //pop x & y, push x or y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(get<bool>(x) or get<bool>(y));
    }

    else if(instr.opcode() == OpCode::NOT){
        //pop x, push not x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      frame->operand_stack.push(not get<bool>(x));
    }

    else if(instr.opcode() == OpCode::CMPLT){
      //pop x & y, push x < y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(lt(y, x));
    }

    else if(instr.opcode() == OpCode::CMPLE){
      //pop x & y, push x <= y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(le(y, x));
    }

    else if(instr.opcode() == OpCode::CMPGT){
      //pop x & y, push x > y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(gt(y, x));
    }

    else if(instr.opcode() == OpCode::CMPGE){
      //pop x & y, push x >= y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(ge(y, x));
    }

    else if(instr.opcode() == OpCode::CMPEQ){
      //pop x & y, push x == y
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(eq(y, x));
    }

    else if(instr.opcode() == OpCode::CMPNE){
      //pop x & y, push x != y
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(!get<bool>(eq(y, x)));
    }

    //----------------------------------------------------------------------
    // Branching
    //----------------------------------------------------------------------

    else if(instr.opcode() == OpCode::JMP){
      //grab operand
      VMValue i = instr.operand().value();
      ensure_not_null(*frame, i);
      //change pc 
      if(holds_alternative<int>(i)){
        frame->pc = get<int>(i);
      }else{
        error("non int param in JMP", *frame);
      }
    }

    else if(instr.opcode() == OpCode::JMPF){
      //grab operand
      VMValue i = instr.operand().value();
      ensure_not_null(*frame, i);
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      if(get<bool>(x) == false){
        //change pc if false
        if(holds_alternative<int>(i)){
         frame->pc = get<int>(i);
        }else{
          error("non int param in JMPF", *frame);
        }
      }
      frame->operand_stack.pop();
    }
    //----------------------------------------------------------------------
    // Functions
    //----------------------------------------------------------------------

    else if(instr.opcode() == OpCode::CALL){
      //get func name
      VMValue x = instr.operand().value();
      ensure_not_null(*frame, x);
      //new func frame
      shared_ptr<VMFrame> new_frame = make_shared<VMFrame>();
      //set frame info
      new_frame->info = frame_info[get<string>(x)];
      //push frame on call stack
      call_stack.push(new_frame);

      //go through args
      for(int i = 0; i < new_frame->info.arg_count; i++){
        VMValue v = frame->operand_stack.top();
        new_frame->operand_stack.push(v);
        frame->operand_stack.pop();
      }

      //set new frame
      frame = new_frame;
    }

    else if(instr.opcode() == OpCode::RET){
      //get ret val
      VMValue v = frame->operand_stack.top();
      //pop frame
      call_stack.pop();
      //if frame exists, push ret val on op stack
      if(!call_stack.empty()){
        frame = call_stack.top();
        frame->operand_stack.push(v);
      }
      
    }
    //----------------------------------------------------------------------
    // Built in functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::WRITE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      cout << to_string(x);
    }

    else if (instr.opcode() == OpCode::READ) {
      string val = "";
      getline(cin, val);
      frame->operand_stack.push(val);
    }

    else if(instr.opcode() == OpCode::SLEN){
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      int sz = get<string>(x).size();
      frame->operand_stack.push(sz);
    }

    else if(instr.opcode() == OpCode::ALEN){
      //pop x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      int sz = array_heap[get<int>(x)].size();
      //push x.size()
      frame->operand_stack.push(sz);
    }

    else if(instr.opcode() == OpCode::GETC){
      //pop string, int
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      //check that y is in bounds
      if(get<int>(y) >= get<string>(x).size() || get<int>(y) < 0){
        error("out-of-bounds string index", *frame);
      }
      string s;
      s += get<string>(x)[get<int>(y)];
      //push x[y]
      frame->operand_stack.push(s);

    }

    else if(instr.opcode() == OpCode::TOINT){
      int y;
      //pop x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      //get type of x
      if(holds_alternative<string>(x)){
        try{
          y = stoi(get<string>(x));
        }catch(exception &ex){
          error("cannot convert string to int", *frame);
        }
      }else{
        y = int(get<double>(x));
      }
      frame->operand_stack.push(y);

    }

    else if(instr.opcode() == OpCode::TODBL){
      double y;
      //pop x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if(holds_alternative<string>(x)){
        try{

          y = stod(get<string>(x));

        }catch(exception &ex){
          error("cannot convert string to double", *frame);
        }

      }else{ 
        y = double(get<int>(x));
      }
      frame->operand_stack.push(y);
    }

    else if(instr.opcode() == OpCode::TOSTR){
      string y;
      //get x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      if(holds_alternative<int>(x)){
        y = to_string(get<int>(x));
      }else{
        y = to_string(get<double>(x));

      }

      //convert to str and push
      frame->operand_stack.push(y);

    }

    else if(instr.opcode() == OpCode::CONCAT){
      //pop x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      //pop y
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      //push string concat
      frame->operand_stack.push(get<string>(y) + get<string>(x));
    }

    //----------------------------------------------------------------------
    // heap
    //----------------------------------------------------------------------

    else if(instr.opcode() == OpCode::ALLOCS){
      //add to heap
      struct_heap[next_obj_id] = {};
      //push obj id
      frame->operand_stack.push(next_obj_id);
      //inc obj id
      ++next_obj_id;

    }

    else if(instr.opcode() == OpCode::ALLOCC){
      //add to heap
      class_heap[next_obj_id] = {};
      //push obj id
      frame->operand_stack.push(next_obj_id);
      //inc obj id
      ++next_obj_id;
    }

    else if(instr.opcode() == OpCode::ALLOCA){
      //pop off value and sz
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      int sz = get<int>(frame->operand_stack.top());
      ensure_not_null(*frame, sz);
      frame->operand_stack.pop();

      //add to heap
      array_heap[next_obj_id] = vector<VMValue>(sz, x);
      
      //push obj id and incr
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;

    }

    else if(instr.opcode() == OpCode::ADDMEM){
      //get obj id of class
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      //add member to obj
      VMValue mem = instr.operand().value();
      class_heap[get<int>(x)].insert({get<string>(mem), nullptr});

    }

    else if(instr.opcode() == OpCode::ADDMTH){
      //get obj id of class
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      // //add method to object
      // //add method, add new frame info
      // shared_ptr<VMFrame> frame = make_shared<VMFrame>();
      // frame->info = frame_info[get<string>(x)];
      // call_stack.push(frame);
      VMValue mem = instr.operand().value();
      class_heap[get<int>(x)].insert({get<string>(mem), nullptr});

    }

    else if(instr.opcode() == OpCode::SETMEM){
      //pop x and y
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      //ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      //set member
      VMValue mem = instr.operand().value();
      class_heap[get<int>(y)][get<string>(mem)] = x;

    }

    else if(instr.opcode() == OpCode::SETMTH){
      //pop x and y
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      //ensure_not_null(*frame, y);
      frame->operand_stack.pop();

  
      VMValue mem = instr.operand().value();
      class_heap[get<int>(y)][get<string>(mem)] = x;

    }

    else if(instr.opcode() == OpCode::GETMEM){
      //pop x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      
      //push obj(x).mem on stack
      VMValue mem = instr.operand().value();
      frame->operand_stack.push(class_heap[get<int>(x)][get<string>(mem)]);

    }

    else if(instr.opcode() == OpCode::ADDF){
      //get obj id of struct
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      //add field to obj
      VMValue f = instr.operand().value();
      struct_heap[get<int>(x)].insert({get<string>(f), nullptr});

    }

    else if(instr.opcode() == OpCode::SETF){
      //pop x and y
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      //ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      //set field
      VMValue f = instr.operand().value();
      struct_heap[get<int>(y)][get<string>(f)] = x;

    }

    else if(instr.opcode() == OpCode::GETF){
      //pop x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      
      //push obj(x).f on stack
      VMValue f = instr.operand().value();
      frame->operand_stack.push(struct_heap[get<int>(x)][get<string>(f)]);

    }

    else if(instr.opcode() == OpCode::SETI){
      //pop x, y, z
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();

      //check if y < array sz
      if(get<int>(y) >= array_heap[get<int>(z)].size() || get<int>(y) < 0){
        error("out-of-bounds array index " + to_string(get<int>(y)) + " of " + to_string(array_heap[get<int>(z)].size()) , *frame);
      }
      //set array obj
      array_heap[get<int>(z)][get<int>(y)] = x;

    }

    else if(instr.opcode() == OpCode::GETI){
      //pop x and y
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();

      //check that index within bounds
      if(get<int>(x) < 0 || get<int>(x) >= array_heap[get<int>(y)].size()){
        error("out-of-bounds array index", *frame);
      }

      //push obj on stack
      frame->operand_stack.push(array_heap[get<int>(y)][get<int>(x)]);

    }

    
    //----------------------------------------------------------------------
    // special
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::DUP) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(x);
      frame->operand_stack.push(x);      
    }

    else if (instr.opcode() == OpCode::NOP) {
      // do nothing
    }
    
    else {
      error("unsupported operation " + to_string(instr));
    }
  }
}


void VM::ensure_not_null(const VMFrame& f, const VMValue& x) const
{
  if (holds_alternative<nullptr_t>(x))
    error("null reference", f);
}


VMValue VM::add(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) + get<int>(y);
  else
    return get<double>(x) + get<double>(y);
}

VMValue VM::sub(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)){
    return get<int>(x) - get<int>(y);
  }else{
    return get<double>(x) - get<double>(y);
  }
}

VMValue VM::mul(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)){
    return get<int>(x) * get<int>(y);
  }else{
    return get<double>(x) * get<double>(y);
  }
}

VMValue VM::div(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)){
    return get<int>(x) / get<int>(y);
  }else{
    return get<double>(x) / get<double>(y);
  }
}


VMValue VM::eq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) == get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) == get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) == get<string>(y);
  else
    return get<bool>(x) == get<bool>(y);
}

VMValue VM::lt(const VMValue& x, const VMValue& y) const
{ 
  if(holds_alternative<int>(x)){
    return get<int>(x) < get<int>(y);
  }else if(holds_alternative<double>(x)){
    return get<double>(x) < get<double>(y);
  }else{
    return get<string>(x) < get<string>(y);
  }
}

VMValue VM::le(const VMValue& x, const VMValue& y) const
{
  if(holds_alternative<int>(x)){
    return get<int>(x) <= get<int>(y);
  }else if(holds_alternative<double>(x)){
    return get<double>(x) <= get<double>(y);
  }else{
    return get<string>(x) <= get<string>(y);
  }
}

VMValue VM::gt(const VMValue& x, const VMValue& y) const
{
  if(holds_alternative<int>(x)){
    return get<int>(x) > get<int>(y);
  }else if(holds_alternative<double>(x)){
    return get<double>(x) > get<double>(y);
  }else{
    return get<string>(x) > get<string>(y);
  }
}

VMValue VM::ge(const VMValue& x, const VMValue& y) const
{
  if(holds_alternative<int>(x)){
    return get<int>(x) >= get<int>(y);
  }else if(holds_alternative<double>(x)){
    return get<double>(x) >= get<double>(y);
  }else{
    return get<string>(x) >= get<string>(y);
  }
}


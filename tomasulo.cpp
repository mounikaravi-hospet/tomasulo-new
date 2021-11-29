#include <iostream>
using namespace std;
#include <iomanip>
#include <string>
#include <fstream>
//#include <cstring>

#include <Windows.h>

#include <conio.h>

class Instruction_Type
{
public:
    const string static MULT;
    const string static DIVD;
    const string static ADDD;
    const string static SUBD;
    const string static LOAD;
    const string static STORE;
    const string static BNE;
};
const string Instruction_Type::MULT = "MUL";
const string Instruction_Type::DIVD = "DIV";
const string Instruction_Type::ADDD = "ADD";
const string Instruction_Type::SUBD = "SUB";
const string Instruction_Type::LOAD = "LOAD";
const string Instruction_Type::STORE = "STORE";
const string Instruction_Type::BNE = "BNE";

class ReservationStation_Type
{
public:
    const string static ADDD;
    const string static MULT;
};
const string ReservationStation_Type::ADDD = "ADD";
const string ReservationStation_Type::MULT = "MUL";

class LoadStoreBuffer_Type
{
public:
    const string static LOAD;
    const string static STORE;
};
const string LoadStoreBuffer_Type::LOAD = "LOAD";
const string LoadStoreBuffer_Type::STORE = "STORE";

class Instruction_Status
{
public:
    short issue;
    short exe_start;
    short exe_complete;
    short write_back;

    short cycle_remaining;

    Instruction_Status()
    {
        issue = -1;
        exe_start = -1;
        exe_complete = -1;
        write_back = -1;
        cycle_remaining = -1;
    }
};

class Instruction : public Instruction_Status
{
public:
    string inst_type;
    string dest_reg;
    string src_reg_1;
    string src_reg_2;
    int immediate_offset;
    Instruction_Status inst_status;

    Instruction()
    {
        inst_type = "";
        dest_reg = "";
        src_reg_1 = "";
        src_reg_2 = "";

        immediate_offset = -1;
    }
};

class Reservation_Station : public Instruction
{
public:
    string station_name;
    bool isBusy;
    string inst_type;
    string Vj;
    string Vk;
    string Qj;
    string Qk;
    Instruction *inst;

    Reservation_Station()
    {
        isBusy = false;
        inst = nullptr;
    }
};

class Register_Stat
{
public:
    string reg_name;
    string write_unit;
};

class Load_Store_Buf : public Instruction
{
public:
    string buffer_name;
    bool isBusy;
    string reg_loc;
    string func_unit;
    Instruction *inst;

    Load_Store_Buf()
    {
        isBusy = false;
        buffer_name = "";
        reg_loc = "";
        func_unit = "";
        inst = nullptr;
    }
};

void movecursorto(short, short);

class Tomasulo_Algo : public Load_Store_Buf, public Reservation_Station, public Register_Stat
{
public:
    string current_process;
    int cycle_number;
    Load_Store_Buf *load_buf;
    Load_Store_Buf *store_buf;
    Reservation_Station *add_sub_station;
    Reservation_Station *mul_div_station;
    Register_Stat *status;

    int no_of_load_buf;
    int no_of_store_buf;
    int no_of_add_sub_station;
    int no_of_mul_div_station;

    int no_of_register;
    Instruction *instr;
    int no_of_instr;

    int no_of_load_store_cycle;
    int no_of_add_sub_cycle;
    int no_of_mul_cycle;
    int no_of_div_cycle;

    //loading instruction set from input file given by user
    void loadInstructionSet(string file)
    {
        // if (file == "")
        // {
        //      file = "input_file.txt";
        // }

        //can give input to file using ifstream fin (https://www.geeksforgeeks.org/file-handling-c-classes/)
        //read input file now
        //or go through link (https://www.cs.uic.edu/~jbell/CourseNotes/CPlus/FileIO.html)
        ifstream file_read(file);
        if (file_read.fail())
        {
            cerr << "Error---Failed to open " << file << endl;
            exit(-1);
        }
        string file_data;
        while (file_read.peek() != '$')
            file_read.ignore();

        //leave out the first line
        getline(file_read, file_data);

        //no of stations information being loaded
        bool load_complete = false;
        bool store_complete = false;
        bool add_sub_complete = false;
        bool mul_div_complete = false;

        bool _quit_ = false;

        int i = 0;
        while (i < 4)
        {
            file_read >> file_data;
            //cout << file_data << endl;
            if (file_data == "$Total_Add_Sub_Reservation_Stations")
            {
                add_sub_complete = true;
                file_read >> this->no_of_add_sub_station;
                //cout << this->no_of_add_sub_station << endl;
            }
            else if (file_data == "Total_Mul_Div_Reservation_Stations")
            {
                mul_div_complete = true;
                file_read >> this->no_of_mul_div_station;
                //cout << this->no_of_mul_div_station << endl;
            }
            else if (file_data == "Total_Load_Buffers")
            {
                load_complete = true;
                file_read >> this->no_of_load_buf;
                //cout << this->no_of_load_buf << endl;
            }
            else if (file_data == "Total_Store_Buffers")
            {
                store_complete = true;
                file_read >> this->no_of_store_buf;
                //cout << this->no_of_store_buf << endl;
            }
            i++;
        }
        if (!store_complete)
        {
            cout << "-> Information about the number of store buffers missing." << endl;
            _quit_ = true;
        }
        else if (!load_complete)
        {
            cout << "-> Informaton about the number of load buffers missing." << endl;
            _quit_ = true;
        }
        else if (!mul_div_complete)
        {
            cout << "-> Information about the number of Mult Div Reservation Stations missing" << endl;
            _quit_ = true;
        }

        else if (!add_sub_complete)
        {
            cout << "-> Information about the number of Add Sub Reservation Stations missing." << endl;
            _quit_ = true;
        }

        //no of cycles information loading
        bool mul_cycle_comp = false;
        bool div_cycle_comp = false;
        bool add_sub_cycle_comp = false;
        bool load_store_cycle_comp = false;

        while (file_read.peek() != '$')
            file_read.ignore();

        getline(file_read, file_data); //ignore the line containing comments
        for (int i = 0; i < 4; i++)
        {
            file_read >> file_data;
            if (file_data == "Total_Add_Sub_Cycles")
            {
                file_read >> this->no_of_add_sub_cycle;
                add_sub_cycle_comp = true;
                cout << this->no_of_add_sub_cycle << endl;
            }
            else if (file_data == "Total_Mul_Cycles")
            {
                file_read >> this->no_of_mul_cycle;
                mul_cycle_comp = true;
                //cout << this->no_of_mul_cycle << endl;
            }
            else if (file_data == "Total_Div_Cycles")
            {
                file_read >> this->no_of_div_cycle;
                div_cycle_comp = true;
                //cout << this->no_of_div_cycle << endl;
            }
            else if (file_data == "Total_Load_Store_Cycles")
            {
                file_read >> this->no_of_load_store_cycle;
                load_store_cycle_comp = true;
                //cout << this->no_of_load_store_cycle << endl;
            }
        }
        if (!add_sub_cycle_comp)
        {
            cout << "-> Information about the number of add Sub Cycles missing.";
            _quit_ = true;
        }

        if (!mul_cycle_comp)
        {
            cout << "-> Information about the number of mul cycles missing." << endl;
            _quit_ = true;
        }
        else if (!div_cycle_comp)
        {
            cout << "-> Information about the number of div cycles missing." << endl;
            _quit_ = true;
        }
        else if (!load_store_cycle_comp)
        {
            cout << "-> Information about the number of load store cycles missing." << endl;
            _quit_ = true;
        }

        if (_quit_ == true)
        {
            system("pause");
            exit(EXIT_FAILURE);
        }
        this->load_buf = new Load_Store_Buf[this->no_of_load_buf];
        for (int i = 0; i < no_of_load_buf; i++)
        {
            string load_num = std::to_string(i);
            this->load_buf[i].buffer_name = LoadStoreBuffer_Type::LOAD;
            this->load_buf[i].buffer_name.append(load_num);
            //cout << load_buf[i].buffer_name<< endl;
        }

        this->store_buf = new Load_Store_Buf[this->no_of_store_buf];
        for (int i = 0; i < no_of_store_buf; i++)
        {
            string store_num = std::to_string(i);
            this->store_buf[i].buffer_name = LoadStoreBuffer_Type::STORE;
            this->store_buf[i].buffer_name.append(store_num);
            //cout << store_buf[i].buffer_name<< endl;
        }

        this->add_sub_station = new Reservation_Station[this->no_of_add_sub_station];
        for (int i = 0; i < no_of_add_sub_station; i++)
        {
            string add_sub_num = std::to_string(i);
            this->add_sub_station[i].station_name = ReservationStation_Type::ADDD;
            this->add_sub_station[i].station_name.append(add_sub_num);
            //cout << add_sub_station[i].station_name<< endl;
        }

        this->mul_div_station = new Reservation_Station[this->no_of_mul_div_station];
        for (int i = 0; i < no_of_mul_div_station; i++)
        {
            string mul_div_num = std::to_string(i);
            this->mul_div_station[i].station_name = ReservationStation_Type::MULT;
            this->mul_div_station[i].station_name.append(mul_div_num);
            //cout << mul_div_station[i].station_name<< endl;
        }

        while (file_read.peek() != '$')
            file_read.ignore();

        //number of registers loading
        getline(file_read, file_data);
        file_read >> file_data;
        file_read >> this->no_of_register;
        this->status = new Register_Stat[no_of_register];

        for (int i = 0; i < no_of_register; i++)
        {
            string register_num = std::to_string(i);
            this->status[i].reg_name = "F";
            this->status[i].reg_name.append(register_num);
            //cout << status[i].reg_name<< endl;
        }

        //reading instructions from input.txt
        while (file_read.peek() != '$')
            file_read.ignore();

        getline(file_read, file_data);
        file_read >> this->no_of_instr;
        this->instr = new Instruction[no_of_instr];

        for (int i = 0; i < no_of_instr; i++)
        {
            string inst_data;
            file_read >> inst_data;
            //cout << inst_data << endl;
            if (inst_data == Instruction_Type::ADDD)
            {
                this->instr[i].inst_type = Instruction_Type::ADDD;
                file_read >> this->instr[i].dest_reg;
                file_read >> this->instr[i].src_reg_1;
                file_read >> this->instr[i].src_reg_2;
            }
            else if (inst_data == Instruction_Type::SUBD)
            {
                this->instr[i].inst_type = Instruction_Type::SUBD;
                file_read >> this->instr[i].dest_reg;
                file_read >> this->instr[i].src_reg_1;
                file_read >> this->instr[i].src_reg_2;
            }
            else if (inst_data == Instruction_Type::MULT)
            {
                this->instr[i].inst_type = Instruction_Type::MULT;
                file_read >> this->instr[i].dest_reg;
                file_read >> this->instr[i].src_reg_1;
                file_read >> this->instr[i].src_reg_2;
            }
            else if (inst_data == Instruction_Type::DIVD)
            {
                this->instr[i].inst_type = Instruction_Type::DIVD;
                file_read >> this->instr[i].dest_reg;
                file_read >> this->instr[i].src_reg_1;
                file_read >> this->instr[i].src_reg_2;
            }
            else if (inst_data == Instruction_Type::LOAD)
            {
                this->instr[i].inst_type = Instruction_Type::LOAD;
                file_read >> this->instr[i].src_reg_2;
                file_read >> this->instr[i].immediate_offset;
                file_read >> this->instr[i].src_reg_1;
            }
            else if (inst_data == Instruction_Type::STORE)
            {
                this->instr[i].inst_type = Instruction_Type::STORE;
                file_read >> this->instr[i].src_reg_2;
                file_read >> this->instr[i].immediate_offset;
                file_read >> this->instr[i].src_reg_1;
            }
        }
    }

    int Free_Res_Station_For_Store()
    {
        for (int i = 0; i < no_of_store_buf; i++)
        {
            if (store_buf[i].isBusy == false)
                return i;
        }
        return -1;
    }

    int Free_Res_Station_For_AddSub()
    {
        for (int i = 0; i < no_of_add_sub_station; i++)
        {
            if (add_sub_station[i].isBusy == false)
                return i;
        }
        return -1;
    }

    int Free_Res_Station_For_MulDiv()
    {
        for (int i = 0; i < no_of_mul_div_station; i++)
        {
            if (mul_div_station[i].isBusy == false)
                return i;
        }
        return -1;
    }

    int Free_Res_Station_For_Load()
    {
        for (int i = 0; i < no_of_load_buf; i++)
        {
            if (load_buf[i].isBusy == false)
                return i;
        }
        return -1;
    }

    void BroadCast(string val, string name)
    {
        for (int i = 0; i < no_of_load_buf; i++)
        {
            if (load_buf[i].isBusy == false)
                continue;

            if (load_buf[i].func_unit == name)
                this->load_buf[i].func_unit = "";
        }

        for (int i = 0; i < no_of_store_buf; i++)
        {
            if (store_buf[i].isBusy == false)
                continue;

            if (store_buf[i].func_unit == name)
                this->store_buf[i].func_unit = "";
        }

        for (int i = 0; i < no_of_add_sub_station; i++)
        {
            if (add_sub_station[i].isBusy == false)
                continue;

            if (add_sub_station[i].Qj == name)
            {
                this->add_sub_station[i].Qj = "";
                this->add_sub_station[i].Vj = val;
            }

            if (add_sub_station[i].Qk == name)
            {
                this->add_sub_station[i].Qk = "";
                this->add_sub_station[i].Vk = val;
            }
        }

        for (int i = 0; i < no_of_mul_div_station; i++)
        {
            if (mul_div_station[i].isBusy == false)
                continue;

            if (mul_div_station[i].Qj == name)
            {
                this->mul_div_station[i].Qj = "";
                this->mul_div_station[i].Vj = val;
            }

            if (mul_div_station[i].Qk == name)
            {
                this->mul_div_station[i].Qk = "";
                this->mul_div_station[i].Vk = val;
            }
        }
    }

    int Issue_Instruction(int curr_instr)
    {
        int buffer_no;
        string num = std::to_string(curr_instr);

        if (curr_instr < no_of_instr)
        {
            if (instr[curr_instr].inst_type == Instruction_Type::LOAD)
            {
                buffer_no = Free_Res_Station_For_Load();

                if (buffer_no == -1)
                {
                    current_process = current_process + "#Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " not issued due to structural hazard.\n";

                    return -1;
                }

                else
                {
                    load_buf[buffer_no].isBusy = true;
                    load_buf[buffer_no].inst = &this->instr[curr_instr];
                    load_buf[buffer_no].reg_loc = this->instr[curr_instr].src_reg_1;
                    string offset = std::to_string(instr[curr_instr].immediate_offset);
                    load_buf[buffer_no].reg_loc += " + ";
                    load_buf[buffer_no].reg_loc.append(offset);
                    this->instr[curr_instr].inst_status.issue = cycle_number;
                    current_process = current_process + "Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " is issued at load buffer: " + load_buf[buffer_no].buffer_name + "\n";

                    this->instr[curr_instr].inst_status.cycle_remaining = this->no_of_load_store_cycle;
                    //checking RAW hazard
                    int register_no = atoi(&this->instr[curr_instr].src_reg_2.c_str()[1]);
                    this->load_buf[buffer_no].func_unit = this->status[register_no].write_unit;
                    //set status of register being written by load instruction
                    this->status[register_no].write_unit = load_buf[buffer_no].buffer_name;
                }
            }

            else if (instr[curr_instr].inst_type == Instruction_Type::STORE)
            {
                buffer_no = Free_Res_Station_For_Store();

                if (buffer_no == -1)
                {
                    current_process = current_process + "#Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " not issued due to structural hazard.\n";

                    return -1;
                }

                else
                {
                    store_buf[buffer_no].isBusy = true;
                    store_buf[buffer_no].inst = &this->instr[curr_instr];
                    store_buf[buffer_no].reg_loc = this->instr[curr_instr].src_reg_1;
                    string offset = std::to_string(instr[curr_instr].immediate_offset);
                    store_buf[buffer_no].reg_loc += " + ";
                    store_buf[buffer_no].reg_loc.append(offset);
                    this->instr[curr_instr].inst_status.issue = cycle_number;
                    current_process = current_process + "Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " is issued at store buffer: " + store_buf[buffer_no].buffer_name + "\n";

                    this->instr[curr_instr].inst_status.cycle_remaining = this->no_of_load_store_cycle;
                    //checking RAW hazard
                    int register_no = atoi(&this->instr[curr_instr].src_reg_2.c_str()[1]);
                    this->store_buf[buffer_no].func_unit = this->status[register_no].write_unit;
                    //set status of register being written by store instruction
                    this->status[register_no].write_unit = store_buf[buffer_no].buffer_name;
                }
            }

            else if (instr[curr_instr].inst_type == Instruction_Type::ADDD || instr[curr_instr].inst_type == Instruction_Type::SUBD)
            {
                buffer_no = Free_Res_Station_For_AddSub();

                if (buffer_no == -1)
                {
                    current_process = current_process + "#Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " not issued due to structural hazard.\n";

                    return -1;
                }

                else
                {
                    add_sub_station[buffer_no].isBusy = true;
                    add_sub_station[buffer_no].inst = &this->instr[curr_instr];
                    this->instr[curr_instr].inst_status.issue = cycle_number;
                    current_process = current_process + "Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " is issued at reservation station: " + add_sub_station[buffer_no].station_name + "\n";

                    this->instr[curr_instr].inst_status.cycle_remaining = this->no_of_add_sub_cycle;
                    this->add_sub_station[buffer_no].inst_type = this->instr[curr_instr].inst_type;
                    //checking RAW hazard
                    int register_no = atoi(&this->instr[curr_instr].src_reg_1.c_str()[1]);
                    this->add_sub_station[buffer_no].Qj = this->status[register_no].write_unit;

                    register_no = atoi(&this->instr[curr_instr].src_reg_2.c_str()[1]);
                    this->add_sub_station[buffer_no].Qk = this->status[register_no].write_unit;

                    if (this->add_sub_station[buffer_no].Qj == "")
                    {
                        this->add_sub_station[buffer_no].Vj = "R(" + this->instr[curr_instr].src_reg_1 + ")";
                    }
                    if (this->add_sub_station[buffer_no].Qk == "")
                    {
                        this->add_sub_station[buffer_no].Vk = "R(" + this->instr[curr_instr].src_reg_2 + ")";
                    }

                    register_no = atoi(&this->instr[curr_instr].dest_reg.c_str()[1]);
                    //set status of register being written by add or sub instruction
                    this->status[register_no].write_unit = add_sub_station[buffer_no].station_name;
                }
            }
            else if (instr[curr_instr].inst_type == Instruction_Type::MULT || instr[curr_instr].inst_type == Instruction_Type::DIVD)
            {
                buffer_no = Free_Res_Station_For_MulDiv();

                if (buffer_no == -1)
                {
                    current_process = current_process + "#Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " not issued due to structural hazard.\n";

                    return -1;
                }

                else
                {
                    mul_div_station[buffer_no].isBusy = true;
                    mul_div_station[buffer_no].inst = &this->instr[curr_instr];
                    this->instr[curr_instr].inst_status.issue = cycle_number;
                    current_process = current_process + "Instruction Number: ";
                    current_process.append(num);
                    current_process = current_process + " is issued at reservation station: " + mul_div_station[buffer_no].station_name + "\n";

                    if (instr[curr_instr].inst_type == Instruction_Type::MULT)
                        this->instr[curr_instr].inst_status.cycle_remaining = this->no_of_mul_cycle;
                    else
                        this->instr[curr_instr].inst_status.cycle_remaining = this->no_of_div_cycle;

                    this->mul_div_station[buffer_no].inst_type = this->instr[curr_instr].inst_type;
                    //checking RAW hazard
                    int register_no = atoi(&this->instr[curr_instr].src_reg_1.c_str()[1]);
                    this->mul_div_station[buffer_no].Qj = this->status[register_no].write_unit;

                    register_no = atoi(&this->instr[curr_instr].src_reg_2.c_str()[1]);
                    this->mul_div_station[buffer_no].Qk = this->status[register_no].write_unit;

                    if (this->mul_div_station[buffer_no].Qj == "")
                    {
                        this->mul_div_station[buffer_no].Vj = "R(" + this->instr[curr_instr].src_reg_1 + ")";
                    }
                    if (this->mul_div_station[buffer_no].Qk == "")
                    {
                        this->mul_div_station[buffer_no].Vk = "R(" + this->instr[curr_instr].src_reg_2 + ")";
                    }

                    register_no = atoi(&this->instr[curr_instr].dest_reg.c_str()[1]);
                    //set status of register being written by mul or div instruction
                    this->status[register_no].write_unit = mul_div_station[buffer_no].station_name;
                }
            }
        }
        return 0;
    }

    void Writing_Back()
    {
        for (int i = 0; i < no_of_load_buf; i++)
        {
            int static number;
            if (this->load_buf[i].isBusy == false)
                continue;
            if (this->load_buf[i].inst->inst_status.cycle_remaining != 0)
                continue; //instruction is still being executed
            if (this->load_buf[i].inst->inst_status.exe_complete == cycle_number)
                continue; //instruction completed at this cycle number so can't write back yet

            this->load_buf[i].inst->inst_status.write_back = cycle_number;
            current_process += "#Instruction at load buffer " + load_buf[i].buffer_name + " is written back.\n";
            int reg_no = atoi(&this->load_buf[i].inst->src_reg_2.c_str()[1]);

            if (this->status[reg_no].write_unit == this->load_buf[i].buffer_name)
                this->status[reg_no].write_unit = "";

            this->load_buf[i].isBusy = false;
            this->load_buf[i].reg_loc = "";
            this->load_buf[i].inst = nullptr;
            string char_num = std::to_string(number);
            string value = "M(A";
            value.append(char_num);
            value += ")";
            BroadCast(value, this->load_buf[i].buffer_name);
            number++;
        }

        for (int i = 0; i < no_of_store_buf; i++)
        {
            if (this->store_buf[i].isBusy == false)
                continue;
            if (this->store_buf[i].inst->inst_status.cycle_remaining != 0)
                continue; //instruction is still being executed
            if (this->store_buf[i].inst->inst_status.exe_complete == cycle_number)
                continue; //instruction completed at this cycle number so can't write back yet

            this->store_buf[i].inst->inst_status.write_back = cycle_number;
            current_process += "#Instruction at store buffer " + store_buf[i].buffer_name + " is written back.\n";

            this->store_buf[i].isBusy = false;
            this->store_buf[i].reg_loc = "";
            this->store_buf[i].inst = nullptr;
        }

        for (int i = 0; i < no_of_add_sub_station; i++)
        {
            int static number;
            if (this->add_sub_station[i].isBusy == false)
                continue;
            if (this->add_sub_station[i].inst->inst_status.cycle_remaining != 0)
                continue; //instruction is still being executed
            if (this->add_sub_station[i].inst->inst_status.exe_complete == cycle_number)
                continue; //instruction completed at this cycle number so can't write back yet

            this->add_sub_station[i].inst->inst_status.write_back = cycle_number;
            current_process += "#Instruction at Reservation station " + add_sub_station[i].station_name + " is written back.\n";
            int reg_no = atoi(&this->add_sub_station[i].inst->dest_reg.c_str()[1]);

            if (this->status[reg_no].write_unit == this->add_sub_station[i].station_name)
                this->status[reg_no].write_unit = "";

            this->add_sub_station[i].isBusy = false;
            this->add_sub_station[i].inst_type = "";
            this->add_sub_station[i].Qj = "";
            this->add_sub_station[i].Qk = "";
            this->add_sub_station[i].Vj = "";
            this->add_sub_station[i].Vk = "";
            this->add_sub_station[i].inst = nullptr;
            string char_num = std::to_string(number);
            string value = "V";
            value.append(char_num);
            BroadCast(value, this->add_sub_station[i].station_name);
            number++;
        }

        for (int i = 0; i < no_of_mul_div_station; i++)
        {
            int static number;
            if (this->mul_div_station[i].isBusy == false)
                continue;
            if (this->mul_div_station[i].inst->inst_status.cycle_remaining != 0)
                continue; //instruction is still being executed
            if (this->mul_div_station[i].inst->inst_status.exe_complete == cycle_number)
                continue; //instruction completed at this cycle number so can't write back yet

            this->add_sub_station[i].inst->inst_status.write_back = cycle_number;
            current_process += "#Instruction at Reservation station " + mul_div_station[i].station_name + " is written back.\n";
            int reg_no = atoi(&this->mul_div_station[i].inst->dest_reg.c_str()[1]);

            if (this->status[reg_no].write_unit == this->mul_div_station[i].station_name)
                this->status[reg_no].write_unit = "";

            this->mul_div_station[i].isBusy = false;
            this->mul_div_station[i].inst_type = "";
            this->mul_div_station[i].Qj = "";
            this->mul_div_station[i].Qk = "";
            this->mul_div_station[i].Vj = "";
            this->mul_div_station[i].Vk = "";
            this->mul_div_station[i].inst = nullptr;
            string char_num = std::to_string(number);
            string value = "V";
            value.append(char_num);
            BroadCast(value, this->mul_div_station[i].station_name);
            number++;
        }
    }

    void execute()
    {
        int i = 0;
        while (i < no_of_load_buf)
        {
            if (this->load_buf[i].isBusy == false)
                continue;
            if (this->load_buf[i].func_unit != "")
                continue;
            if (this->load_buf[i].inst->inst_status.exe_start == -1)
            {
                if (this->load_buf[i].inst->inst_status.issue == cycle_number)
                    continue; //instruction is issued in current cycle so execution doesn't start yet

                //execution then starts
                this->load_buf[i].inst->inst_status.exe_start = cycle_number;
                this->load_buf[i].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1

                if (this->load_buf[i].inst->inst_status.cycle_remaining == 0)
                {
                    //we are done with the execution of the instruction
                    this->load_buf[i].inst->inst_status.exe_complete = cycle_number;
                    //display this on the screen
                    current_process = current_process + "#Instruction at load buffer " + load_buf[i].buffer_name + " has completed execution.\n";
                    continue;
                }
                else
                {
                    current_process = current_process + "#Instruction at load buffer " + load_buf[i].buffer_name + " has started its execution.\n";
                    continue;
                }
            }

            if (this->load_buf[i].inst->inst_status.cycle_remaining != 0)
            {
                this->load_buf[i].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1 for every cycle until it completes
            }
            else
                continue;

            if (this->load_buf[i].inst->inst_status.cycle_remaining == 0)
            {
                //we are done with the execution of the instruction
                this->load_buf[i].inst->inst_status.exe_complete = cycle_number;
                //display this on the screen
                current_process = current_process + "#Instruction at load buffer " + load_buf[i].buffer_name + " has completed execution.\n";
                continue;
            }
            else
            {
                current_process = current_process + "#Instruction at load buffer " + load_buf[i].buffer_name + " has started its execution.\n";
                continue;
            }
            i++;
        }

        //similarly for store buffer
        int j = 0;
        while (j < no_of_store_buf)
        {
            if (this->store_buf[j].isBusy == false)
                continue;
            if (this->store_buf[j].func_unit != "")
                continue;
            if (this->store_buf[j].inst->inst_status.exe_start == -1)
            {
                if (this->store_buf[j].inst->inst_status.issue == cycle_number)
                    continue; //instruction is issued in current cycle so execution doesn't start yet

                //execution then starts
                this->store_buf[j].inst->inst_status.exe_start = cycle_number;
                this->store_buf[j].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1

                if (this->store_buf[j].inst->inst_status.cycle_remaining == 0)
                {
                    //we are done with the execution of the instruction
                    this->store_buf[j].inst->inst_status.exe_complete = cycle_number;
                    //display this on the screen
                    current_process = current_process + "#Instruction at store buffer " + store_buf[j].buffer_name + " has completed execution.\n";
                    continue;
                }
                else
                {
                    current_process = current_process + "#Instruction at store buffer " + store_buf[j].buffer_name + " has started its execution.\n";
                    continue;
                }
            }

            if (this->store_buf[j].inst->inst_status.cycle_remaining != 0)
            {
                this->store_buf[j].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1 for every cycle until it completes
            }
            else
                continue;

            if (this->store_buf[j].inst->inst_status.cycle_remaining == 0)
            {
                //we are done with the execution of the instruction
                this->store_buf[j].inst->inst_status.exe_complete = cycle_number;
                //display this on the screen
                current_process = current_process + "#Instruction at store buffer " + store_buf[j].buffer_name + " has completed execution.\n";
                continue;
            }
            else
            {
                current_process = current_process + "#Instruction at store buffer " + store_buf[j].buffer_name + " has started its execution.\n";
                continue;
            }
            j++;
        }

        //similarly for add/sub reservation station
        int k = 0;
        while (k < no_of_add_sub_station)
        {
            if (this->add_sub_station[k].isBusy == false)
                continue;
            if (this->add_sub_station[k].Qj != "" || this->add_sub_station[k].Qk != "")
                continue; //Due to RAW hazard the execution is not started
            if (this->add_sub_station[k].inst->inst_status.exe_start == -1)
            {
                if (this->add_sub_station[k].inst->inst_status.issue == cycle_number)
                    continue; //instruction is issued in current cycle so execution doesn't start yet

                //execution then starts
                this->add_sub_station[k].inst->inst_status.exe_start = cycle_number;
                this->add_sub_station[k].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1

                if (this->add_sub_station[k].inst->inst_status.cycle_remaining == 0)
                {
                    //we are done with the execution of the instruction
                    this->add_sub_station[k].inst->inst_status.exe_complete = cycle_number;
                    //display this on the screen
                    current_process = current_process + "#Instruction at reservation station " + add_sub_station[k].station_name + " has completed execution.\n";
                    continue;
                }
                else
                {
                    current_process = current_process + "#Instruction at reservation station " + add_sub_station[k].station_name + " has started its execution.\n";
                    continue;
                }
            }

            if (this->add_sub_station[k].inst->inst_status.cycle_remaining != 0)
            {
                this->add_sub_station[k].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1 for every cycle until it completes
            }
            else
                continue;

            if (this->add_sub_station[k].inst->inst_status.cycle_remaining == 0)
            {
                //we are done with the execution of the instruction
                this->add_sub_station[k].inst->inst_status.exe_complete = cycle_number;
                //display this on the screen
                current_process = current_process + "#Instruction at reservation station " + add_sub_station[k].station_name + " has completed execution.\n";
                continue;
            }
            else
            {
                current_process = current_process + "#Instruction at reservation station " + add_sub_station[k].station_name + " has started its execution.\n";
                continue;
            }
            k++;
        }

        //similarly for mul/div instruction
        int l = 0;
        while (l < no_of_mul_div_station)
        {
            if (this->mul_div_station[l].isBusy == false)
                continue;
            if (this->mul_div_station[l].Qj != "" || this->mul_div_station[l].Qk != "")
                continue; //Due to RAW hazard the execution is not started
            if (this->mul_div_station[l].inst->inst_status.exe_start == -1)
            {
                if (this->mul_div_station[l].inst->inst_status.issue == cycle_number)
                    continue; //instruction is issued in current cycle so execution doesn't start yet

                //execution then starts
                this->mul_div_station[l].inst->inst_status.exe_start = cycle_number;
                this->mul_div_station[l].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1

                if (this->mul_div_station[l].inst->inst_status.cycle_remaining == 0)
                {
                    //we are done with the execution of the instruction
                    this->mul_div_station[l].inst->inst_status.exe_complete = cycle_number;
                    //display this on the screen
                    current_process = current_process + "#Instruction at reservation station " + mul_div_station[l].station_name + " has completed execution.\n";
                    continue;
                }
                else
                {
                    current_process = current_process + "#Instruction at reservation station " + mul_div_station[l].station_name + " has started its execution.\n";
                    continue;
                }
            }

            if (this->mul_div_station[l].inst->inst_status.cycle_remaining != 0)
            {
                this->mul_div_station[l].inst->inst_status.cycle_remaining--; //after execution starts number of cycle decrements by 1 for every cycle until it completes
            }
            else
                continue;

            if (this->mul_div_station[l].inst->inst_status.cycle_remaining == 0)
            {
                //we are done with the execution of the instruction
                this->mul_div_station[l].inst->inst_status.exe_complete = cycle_number;
                //display this on the screen
                current_process = current_process + "#Instruction at reservation station " + mul_div_station[l].station_name + " has completed execution.\n";
                continue;
            }
            else
            {
                current_process = current_process + "#Instruction at reservation station " + mul_div_station[l].station_name + " has started its execution.\n";
                continue;
            }
            l++;
        }
    }

    void simulation()
    {
        //simulate the algorithm
        int curr_inst_number_to_issue = 0;
        int curr_cycle = 0;

        while (1)
        {
            movecursorto(0, 0);
            cout << "Current Cycle is: " << curr_cycle;
            display();
            char ch;
            cout << "press any key and hit enter......";
            cin >> ch;
            curr_cycle += 1;

            system("cls");
            current_process = "";

            int flag = Issue_Instruction(curr_inst_number_to_issue);

            if (flag == -1)
            {
                curr_inst_number_to_issue += 1;

                execute();
                Writing_Back();
            }
        }
    }
    void display()
    {
        int y = 2;
        movecursorto(2, y);
        cout << "Instructions:";
        movecursorto(27, y);
        cout << "  Issue   Start  Finish  Write";

        movecursorto(27, y + 1);
        cout << "__________________________________";

        int j = 0;
        for (int i = 0; i < no_of_instr; i++)
        {
            char chr[10];
            _itoa_s(i, chr, 10);
            strcat_s(chr, ".");

            movecursorto(2, j + y + 2);
            if (instr[i].inst_type == Instruction_Type::LOAD || instr[i].inst_type == Instruction_Type::STORE)
            {
                cout << std::left << setw(3) << chr << std::left << setw(5) << instr[i].inst_type << setfill(' ') << std::right << setw(4) << instr[i].src_reg_2 << setw(5) << instr[i].immediate_offset << "+" << setw(5) << instr[i].src_reg_1;
            }
            else
            {

                cout << std::left << setw(3) << chr << std::left << setw(5) << instr[i].inst_type << std::right << setw(4) << instr[i].dest_reg << setw(5) << instr[i].src_reg_1 << setw(6) << instr[i].src_reg_2;
            }

            movecursorto(26, j + y + 2);
            j++;

            char issue_instruction[10];
            char start_execution[10];
            char complete_execution[10];
            char write_instrustion[10];

            _itoa_s(this->instr[i].inst_status.issue, issue_instruction, 10);
            _itoa_s(this->instr[i].inst_status.exe_start, start_execution, 10);
            _itoa_s(this->instr[i].inst_status.exe_complete, complete_execution, 10);
            _itoa_s(this->instr[i].inst_status.write_back, write_instrustion, 10);

            // issue_instruction = std::to_string(this->instr[i].inst_status.issue);

            cout << "|" << std::right << setw(7) << (instr[i].inst_status.issue == -1 ? "" : issue_instruction) << "|" << setw(7) << (instr[i].inst_status.exe_start == -1 ? "" : start_execution) << "|" << setw(7) << (instr[i].inst_status.exe_complete == -1 ? "" : complete_execution) << "|" << setw(10) << (instr[i].inst_status.write_back == -1 ? "" : write_instrustion) << "|";

            movecursorto(26, j + y + 2);

            cout << "|_______|_______|_______|__________|";
            j++;
        }
        // load store buff
        movecursorto(70, y);
        cout << "Load_Store Buffer: "
             << "Busy      Address     FU";
        y++;
        movecursorto(90, y);
        cout << "_____________________________";
        for (int i = 0; i < no_of_load_buf; i++)
        {
            y++;
            movecursorto(78, y);
            cout << setw(10) << std::right << load_buf[i].buffer_name;
            cout << " ";
            cout << "|" << std::right << setw(7) << (load_buf[i].isBusy == true ? "yes" : "no") << "|" << setw(10) << load_buf[i].reg_loc << "|" << setw(10) << load_buf[i].func_unit << "|";
            if (load_buf[i].inst != nullptr)
            {
                cout << setw(3) << load_buf[i].inst->inst_status.cycle_remaining;
            }
            y++;
            movecursorto(89, y);
            cout << "|_______|__________|__________|" << endl;
        }
        // Res Stations
        y = (y > j ? y : j);
        y += 6;
        movecursorto(4, y);
        cout << "Reservation Stations:";
        y++;
        movecursorto(21, y);
        cout << " Name"
             << "   Busy"
             << "    Op"
             << "       Vj"
             << "       Vk"
             << "       Qj"
             << "       Qk";

        y++;
        movecursorto(28, y);
        cout << "_________________________________________________";
        // add sub
        for (int i = 0; i < no_of_add_sub_station; i++)
        {
            y++;
            movecursorto(19, y);
            cout << std::right << setw(7) << add_sub_station[i].station_name << " |" << setw(5) << (add_sub_station[i].isBusy == true ? "yes" : "no") << "|" << setw(7) << add_sub_station[i].inst_type << "|" << setw(8) << add_sub_station[i].Vj << "|" << setw(8) << add_sub_station[i].Vk << "|" << setw(8) << add_sub_station[i].Qj << "|" << setw(8) << add_sub_station[i].Qk << "|";
            if (add_sub_station[i].inst != nullptr)
            {
                cout << setw(3) << add_sub_station[i].inst->inst_status.cycle_remaining;
            }
            y++;
            movecursorto(27, y);
            cout << "|_____|_______|________|________|________|________|";
        }

        // mul div
        for (int i = 0; i < no_of_mul_div_station; i++)
        {
            y++;
            movecursorto(19, y);
            cout << std::right << setw(7) << mul_div_station[i].station_name << " |" << setw(5) << (mul_div_station[i].isBusy == true ? "yes" : "no") << "|" << setw(7) << mul_div_station[i].inst_type << "|" << setw(8) << mul_div_station[i].Vj << "|" << setw(8) << mul_div_station[i].Vk << "|" << setw(8) << mul_div_station[i].Qj << "|" << setw(8) << mul_div_station[i].Qk << "|";

            if (mul_div_station[i].inst != nullptr)
            {
                cout << setw(3) << mul_div_station[i].inst->inst_status.cycle_remaining;
            }

            y++;
            movecursorto(27, y);
            cout << "|_____|_______|________|________|________|________|";
        }

        y += 3;
        int x = 19;

        for (int i = 0; i < no_of_register; i++)
        {
            movecursorto(x, y);
            cout << std::right << setw(5) << status[i].reg_name;

            movecursorto(x + 1, y + 1);

            cout << "________";
            movecursorto(x, y + 2);
            cout << "|" << setw(5) << status[i].write_unit;

            movecursorto(x, y + 3);
            cout << "|________";
            x += 5 + status[i].reg_name.length() + 1;
        }
        movecursorto(x, y + 2);
        cout << "|";
        movecursorto(x, y + 3);
        cout << "|";

        cout << endl
             << endl;
        cout << "What's happening currently: " << endl
             << current_process;
    }
};

int main(int argc, char *argv[])
{
    // cout<<"Hey";

    system("pause");
    // cout<<"Hello";
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;  // Width of each character in the font
    cfi.dwFontSize.Y = 18; // Height
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"Consolas"); // Choose your font
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

    SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
    COORD newSize = {1000, 1000};
    //SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), newSize);

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);

    system("cls");
    Tomasulo_Algo tom;
    tom.loadInstructionSet("input_file.txt");
    tom.simulation();
    return 0;
}

void movecursorto(short x, short y)
{
    COORD c = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

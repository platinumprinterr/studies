#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

// Constants and type definitions
#define CHARBUFFER 50
#define ADDRBUFFER 150
#define BUFFER 256
#define IBAN_LENGTH 8  
#define PESEL_LENGTH 11
#define ID_LEN 4
#define BALANCE_SIZE_C 12
#define DEBT_SIZE_C 12
#define PRECISION 2
#define LINE_LENGTH 120 
#define DATA_FILE "accounts.dat"
#define COUNTRY "PL"
#define BANK_CODE "1234"
#define CASH_MIN 0.0
#define CASH_MAX 999999.99
#define LOAN_MAX 50000.0
#define MONTHS_OF_PAYMENT 12

typedef enum {
    INPUT_SUCCESS = 0,
    INPUT_GO_BACK = 1,
    INPUT_ERROR = 2
} InputStatus_t;

typedef char Fixed_string[CHARBUFFER];
typedef char Address[ADDRBUFFER];
typedef char PESEL[PESEL_LENGTH + 1];
typedef char IBAN[IBAN_LENGTH + 1];

typedef struct
{
    uint32_t id;
    IBAN account_number;
    Fixed_string first_name;
    Fixed_string last_name;
    Address address;
    PESEL pesel_number;
    double balance;
    double debt;
} Account_t;

typedef struct {
    char *headers[8];
    Account_t *accounts;
    int count;
} AccountList_t;

char quit_flag = 0;

// Function declarations
void printActions();
void printModifyingOptions();
void printDisplayOptions();
void printOutOfRange();
void printSuccess();
void printAbort();
void waitingForReturn();
void printSearchOptions();
void printHelpMenu();
void printErrorAndWait(const char* error_msg);
InputStatus_t getString(char *str, int size, const char *msg, bool clear);
bool checkLetters(char *string);
bool noLetters(char *string);
bool checkDigits(char *string);
InputStatus_t getDouble(double *value, double min, double max, const char *msg);
void printAccount(Account_t acc);
void printLine();
void printAllList();
void printListHeader();
void printAccounts(Fixed_string key, bool (*condition)(Account_t ref, Fixed_string key));

bool findName(Account_t ref, Fixed_string key);
bool findSurname(Account_t ref, Fixed_string key);
bool findAddress(Account_t ref, Fixed_string key);
bool findPESEL(Account_t ref, Fixed_string key);
bool findAccountNumber(Account_t ref, Fixed_string key);
InputStatus_t getSearchKey(char *search_key, short len);
void searchList();

void transferMoney();
void makeDeposit();
void makeWithdrawal();
void takeLoan();
void payDebt();

void perfromTransferUpdate(Account_t accs[]);
void performOtherUpdate(Account_t acc);
bool confirmation(Account_t accounts[], bool is_transfer);
void updateAccount(Account_t updated);
void updateTransfer(Account_t source, Account_t destination);
InputStatus_t findAccount(const char *msg, bool *found, Account_t *account);

InputStatus_t getPESEL(Account_t *new);
InputStatus_t getName(Account_t *new);
InputStatus_t getLocation(Account_t *new);
InputStatus_t getBalance(Account_t *new);
InputStatus_t getDebtInfo(Account_t *new);
void generateIBAN(Account_t *new);
bool isIBANoverlapping(IBAN check_val);
uint32_t getLastID();
void createAccount();

int getAction();
void chooseAction();
void chooseModifyingOperation();
void chooseDisplayOperation();

//----------------> FUNCTION IMPLEMENTATIONS <----------------

void printErrorAndWait(const char* error_msg)
{
    printf("ERROR: %s\n", error_msg);
    printf("Press Enter to try again...\n");
    while (getchar() != '\n');
}

// Search functions
bool findName(Account_t ref, Fixed_string key)
{
    return strstr(ref.first_name, key) != NULL;
}

bool findSurname(Account_t ref, Fixed_string key)
{
    return strstr(ref.last_name, key) != NULL;
}

bool findAddress(Account_t ref, Fixed_string key)
{
    return strstr(ref.address, key) != NULL;
}

bool findPESEL(Account_t ref, Fixed_string key)
{
    return strstr(ref.pesel_number, key) != NULL;
}

bool findAccountNumber(Account_t ref, Fixed_string key)
{
    return strstr(ref.account_number, key) != NULL;
}

InputStatus_t getSearchKey(char *search_key, short len)
{
    return getString(search_key, len, "Enter search key (or 'r' to return): ", true);
}

void searchList()
{
    printSearchOptions();
    bool (*searchFun)(Account_t ref, Fixed_string key);
    short len = CHARBUFFER;
    Fixed_string search_type;
    Address search_key;
    
    while (1)
    {
        if (getString(search_type, CHARBUFFER, "Enter search type (or 'r' to return): ", false) == INPUT_GO_BACK)
            return;
            
        if (strcmp(search_type, "account") == 0)
        {
            searchFun = &findAccountNumber;
            len = IBAN_LENGTH + 1;
            break;
        }
        else if (strcmp(search_type, "name") == 0)
        {
            searchFun = &findName;
            break;
        }
        else if (strcmp(search_type, "surname") == 0)
        {
            searchFun = &findSurname;
            break;
        }
        else if (strcmp(search_type, "address") == 0)
        {
            searchFun = &findAddress;
            len = ADDRBUFFER + 1;
            break;
        }
        else if (strcmp(search_type, "pesel") == 0)
        {
            searchFun = &findPESEL;
            len = PESEL_LENGTH + 1;
            break;
        }
        else
        {
            printErrorAndWait("Invalid search type. Valid options: account, name, surname, address, pesel");
        }
    }
    
    if (getSearchKey(search_key, len) == INPUT_GO_BACK)
        return;
        
    printAccounts(search_key, searchFun);
}

// Prompt functions
void printActions()
{
    system("clear");
    printf("Choose what you want to do\n");
    printf("1. Accounts modifications\n");
    printf("2. Accounts listing\n");
    printf("3. Help\n");
    printf("4. Quit program\n");
}

void printModifyingOptions()
{
    system("clear");
    printf("Choose what you want to do\n");
    printf("1. Create a new account\n");
    printf("2. Make a deposit\n");
    printf("3. Make a withdrawal\n");
    printf("4. Make a money transfer\n");
    printf("5. Take a loan\n");
    printf("6. Pay a debt\n");
}

void printDisplayOptions()
{
    system("clear");
    printf("Choose what you want to do\n");
    printf("1. List all accounts\n");
    printf("2. Search an account\n");
}

void printOutOfRange()
{
    system("clear");
    printf("Value overflow, operation terminated\n");
    waitingForReturn();
}

void printSuccess()
{
    system("clear");
    printf("Operation successful\n");
    waitingForReturn();
}

void printAbort()
{
    system("clear");
    printf("Operation aborted\n");
    waitingForReturn();
}

void waitingForReturn()
{
    char quit;
    printf("Press 'r' or 'R' to return\n");
    while ((quit = getchar()) != 'r' && quit != 'R')
        ;
    while (getchar() != '\n')
        ;
}

void printSearchOptions()
{
    system("clear");
    printf("Enter by what you want to search\n");
    printf("account\t-\taccount number\n");
    printf("name\t-\tfirst name\n");
    printf("surname\t-\tlast name\n");
    printf("address\t-\taddress\n");
    printf("pesel\t-\tPESEL number\n");
}

void printHelpMenu()
{
    system("clear");
    printf("Simple Bank Program:\n"
           "- Create accounts, deposits, withdrawals, transfers, loans, debt payments\n"
           "- List or search accounts\n"
           "- Use number keys + Enter to select actions\n"
           "- Type 'r' anytime to return to previous menu\n");
    waitingForReturn();
}

InputStatus_t getString(char *str, int size, const char *msg, bool clear)
{
    char buffer[BUFFER];
    while (1)
    {
        if (clear)
            system("clear");
        printf("%s", msg);
        if (fgets(buffer, BUFFER, stdin) == NULL) {
            printErrorAndWait("Failed to read input");
            continue;
        }
        
        if (strlen(buffer) >= BUFFER - 1 && buffer[BUFFER - 2] != '\n')
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            printErrorAndWait("Input too long, please enter shorter text");
            continue;
        }
        
        if (strlen(buffer) == 2 && (buffer[0] == 'r' || buffer[0] == 'R') && buffer[1] == '\n')
        {
            return INPUT_GO_BACK;
        }
        
        if (strlen(buffer) > size)
        {
            printErrorAndWait("Input too long for this field");
            continue;
        }
        
        if (strlen(buffer) <= 1)
        {
            printErrorAndWait("Input cannot be empty");
            continue;
        }
        
        break;
    }
    
    strncpy(str, buffer, size - 1);
    str[size - 1] = '\0'; 
    str[strcspn(str, "\n")] = '\0';
    
    return INPUT_SUCCESS;
}

bool checkLetters(char *string)
{
    if (string == NULL || strlen(string) == 0)
        return false;
        
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (!isalpha(string[i]) && string[i] != ' ')
            return false;
    }
    return true;
}

bool noLetters(char *string)
{
    if (string == NULL)
        return true;
        
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (isalpha(string[i]))
            return false;
    }
    return true;
}

bool checkDigits(char *string)
{
    if (string == NULL || strlen(string) == 0)
        return false;
        
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (!isdigit(string[i]))
            return false;
    }
    return true;
}

InputStatus_t getDouble(double *value, double min, double max, const char *msg)
{
    char buffer[CHARBUFFER];
    char message[BUFFER];
    sprintf(message, "Enter value of %s (%.2f - %.2f, or 'r' to return): ", msg, min, max);
    
    while (1)
    {
        InputStatus_t status = getString(buffer, CHARBUFFER, message, true);
        if (status == INPUT_GO_BACK)
            return INPUT_GO_BACK;
        if (status == INPUT_ERROR)
            return INPUT_ERROR;
            
        char *endptr;
        *value = strtod(buffer, &endptr);
        
        if (*endptr != '\0' || endptr == buffer) {
            printErrorAndWait("Invalid number format, please enter a valid decimal number");
            continue;
        }
        
        if (*value < min || *value > max) {
            char error_msg[BUFFER];
            sprintf(error_msg, "Value must be between %.2f and %.2f", min, max);
            printErrorAndWait(error_msg);
            continue;
        }
        
        break;
    }

    return INPUT_SUCCESS;
}

void printAccount(Account_t acc)
{
    printf("| %4u | %-8s | %-15s | %-15s | %-30s | %-11s | %10.2f | %10.2f |\n", 
           acc.id, 
           acc.account_number, 
           acc.first_name, 
           acc.last_name, 
           acc.address, 
           acc.pesel_number, 
           acc.balance, 
           acc.debt);
}

void printLine()
{
    for (int i = 0; i < LINE_LENGTH; i++)
    {
        printf("-");
    }
    printf("\n");
}

void printAllList()
{
    printAccounts(NULL, NULL);
}

void printListHeader()
{
    printf("| %4s | %-8s | %-15s | %-15s | %-30s | %-11s | %10s | %10s |\n",
           "ID", 
           "IBAN", 
           "First Name", 
           "Last Name", 
           "Address",
           "PESEL", 
           "Balance",
           "Debt");
}

void printAccounts(Fixed_string key, bool (*condition)(Account_t ref, Fixed_string key))
{
    FILE *print_f = fopen(DATA_FILE, "rb");
    if (print_f == NULL)
    {
        printf("No accounts found or error opening file!\n");
        waitingForReturn();
        return;
    }
    
    Account_t print;
    bool found_any = false;
    system("clear");
    printLine();
    printListHeader();
    printLine();
    
    while (fread(&print, sizeof(Account_t), 1, print_f))
    {
        if (condition == NULL || (condition != NULL && condition(print, key)))
        {
            printAccount(print);
            found_any = true;
        }
    }
    
    if (!found_any && condition != NULL)
    {
        printf("| %-110s |\n", "No accounts found matching the search criteria");
    }
    
    printLine();
    fclose(print_f);
    waitingForReturn();
}

// Modification actions
void transferMoney()
{
    bool source_found = false;
    bool destination_found = false;
    Account_t source, destination;
    
    if (findAccount("source ", &source_found, &source) == INPUT_GO_BACK)
        return;
    if (!source_found)
    {
        printErrorAndWait("Source account was not found");
        return;
    }
    
    if (findAccount("destination ", &destination_found, &destination) == INPUT_GO_BACK)
        return;
    if (!destination_found)
    {
        printErrorAndWait("Destination account was not found");
        return;
    }
    
    if (source.id == destination.id)
    {
        printErrorAndWait("Cannot transfer to the same account");
        return;
    }
    
    double transfer;
    if (getDouble(&transfer, 0.01, CASH_MAX, "transfer amount") == INPUT_GO_BACK)
        return;
        
    if (transfer > source.balance)
    {
        char error_msg[BUFFER];
        sprintf(error_msg, "Insufficient funds. Available balance: %.2f", source.balance);
        printErrorAndWait(error_msg);
        return;
    }
    
    if (transfer <= 0)
    {
        printErrorAndWait("Transfer amount must be positive");
        return;
    }
    
    if ((destination.balance + transfer) > CASH_MAX)
    {
        char error_msg[BUFFER];
        sprintf(error_msg, "Transfer would exceed maximum balance limit of %.2f", CASH_MAX);
        printErrorAndWait(error_msg);
        return;
    }
    
    source.balance -= transfer;
    destination.balance += transfer;
    Account_t accs[] = {source, destination};
    perfromTransferUpdate(accs);
}

void makeDeposit()
{
    bool found = false;
    Account_t deposit_acc;
    
    if (findAccount("deposit", &found, &deposit_acc) == INPUT_GO_BACK)
        return;
    if (!found)
    {
        printErrorAndWait("Account was not found");
        return;
    }
    
    double deposit_amount;
    if (getDouble(&deposit_amount, 0.01, CASH_MAX, "deposit amount") == INPUT_GO_BACK)
        return;
        
    if (deposit_amount <= 0)
    {
        printErrorAndWait("Deposit amount must be positive");
        return;
    }
    
    if ((deposit_acc.balance + deposit_amount) > CASH_MAX)
    {
        char error_msg[BUFFER];
        sprintf(error_msg, "Deposit would exceed maximum balance limit of %.2f", CASH_MAX);
        printErrorAndWait(error_msg);
        return;
    }
    
    deposit_acc.balance += deposit_amount;
    performOtherUpdate(deposit_acc);
}

void makeWithdrawal()
{
    bool found = false;
    Account_t withdrawal_acc;
    
    if (findAccount("withdrawal", &found, &withdrawal_acc) == INPUT_GO_BACK)
        return;
    if (!found)
    {
        printErrorAndWait("Account was not found");
        return;
    }
    
    double withdrawal_amount;
    if (getDouble(&withdrawal_amount, 0.01, withdrawal_acc.balance, "withdrawal amount") == INPUT_GO_BACK)
        return;
        
    if (withdrawal_amount <= 0)
    {
        printErrorAndWait("Withdrawal amount must be positive");
        return;
    }
    
    if (withdrawal_amount > withdrawal_acc.balance)
    {
        char error_msg[BUFFER];
        sprintf(error_msg, "Insufficient funds. Available balance: %.2f", withdrawal_acc.balance);
        printErrorAndWait(error_msg);
        return;
    }
    
    withdrawal_acc.balance -= withdrawal_amount;
    performOtherUpdate(withdrawal_acc);
}

void takeLoan()
{
    bool found = false;
    Account_t loan_acc;
    
    if (findAccount("loan", &found, &loan_acc) == INPUT_GO_BACK)
        return;
    if (!found)
    {
        printErrorAndWait("Account was not found");
        return;
    }
    
    double loan;
    if (getDouble(&loan, 0.01, LOAN_MAX, "loan amount") == INPUT_GO_BACK)
        return;
        
    double interest_rate;
    if (getDouble(&interest_rate, 0.0, 1.0, "interest rate (as decimal, e.g., 0.05 for 5%)") == INPUT_GO_BACK)
        return;
    
    if ((loan_acc.balance + loan) > CASH_MAX)
    {
        char error_msg[BUFFER];
        sprintf(error_msg, "Loan would exceed maximum balance limit of %.2f", CASH_MAX);
        printErrorAndWait(error_msg);
        return;
    }
    
    if (loan <= 0)
    {
        printErrorAndWait("Loan amount must be positive");
        return;
    }
    
    loan_acc.balance += loan;
    double total_loan_with_interest = loan * (1 + interest_rate);
    loan_acc.debt += total_loan_with_interest;
    performOtherUpdate(loan_acc);
}

void payDebt()
{
    bool found = false;
    Account_t debt_acc;
    
    if (findAccount("debt payment", &found, &debt_acc) == INPUT_GO_BACK)
        return;
    if (!found)
    {
        printErrorAndWait("Account was not found");
        return;
    }
    
    if (debt_acc.debt <= 0)
    {
        printErrorAndWait("No debt to pay on this account");
        return;
    }
    
    double max_payment = (debt_acc.balance < debt_acc.debt) ? debt_acc.balance : debt_acc.debt;
    double payment_amount;
    if (getDouble(&payment_amount, 0.01, max_payment, "payment amount") == INPUT_GO_BACK)
        return;
    
    if (payment_amount <= 0)
    {
        printErrorAndWait("Payment amount must be positive");
        return;
    }
    
    if (payment_amount > debt_acc.balance)
    {
        char error_msg[BUFFER];
        sprintf(error_msg, "Insufficient funds. Available balance: %.2f", debt_acc.balance);
        printErrorAndWait(error_msg);
        return;
    }
    
    debt_acc.balance -= payment_amount;
    debt_acc.debt -= payment_amount;
    if (debt_acc.debt < 0) debt_acc.debt = 0;
    performOtherUpdate(debt_acc);
}

// File actions
void perfromTransferUpdate(Account_t accs[])
{
    if (confirmation(accs, true))
    {
        updateTransfer(accs[0], accs[1]);
        printSuccess();
    }
    else
    {
        printAbort();
    }
}

void performOtherUpdate(Account_t acc)
{
    if (confirmation(&acc, false))
    {
        updateAccount(acc);
        printSuccess();
    }
    else
    {
        printAbort();
    }
}

bool confirmation(Account_t accounts[], bool is_transfer)
{
    system("clear");
    printLine();
    printListHeader();
    printLine();
    for (int i = 0; i < (is_transfer ? 2 : 1); i++)
    {
        printAccount(accounts[i]);
    }
    printLine();
    printf("Do you want to perform this action?\nIf yes press Y/y -- otherwise anything else\n");
    int action = getchar();
    while (getchar() != '\n')
        ;
    return (action == 'y' || action == 'Y');
}

void updateAccount(Account_t updated)
{
    FILE *update_f = fopen(DATA_FILE, "rb+");
    if (update_f == NULL)
    {
        printErrorAndWait("Error opening file for update");
        return;
    }
    
    Account_t temp;
    bool found = false;
    long position = 0;

    while (fread(&temp, sizeof(Account_t), 1, update_f))
    {
        if (temp.id == updated.id)
        {
            found = true;
            break;
        }
        position++;
    }
    
    if (!found)
    {
        printErrorAndWait("Account not found for update");
        fclose(update_f);
        return;
    }
    
    if (fseek(update_f, position * sizeof(Account_t), SEEK_SET) != 0)
    {
        printErrorAndWait("Error seeking file position");
        fclose(update_f);
        return;
    }
    
    if (fwrite(&updated, sizeof(Account_t), 1, update_f) != 1)
    {
        printErrorAndWait("Error writing to file");
        fclose(update_f);
        return;
    }
    
    fclose(update_f);
}

void updateTransfer(Account_t source, Account_t destination)
{
    updateAccount(source);
    updateAccount(destination);
}

InputStatus_t findAccount(const char *msg, bool *found, Account_t *account)
{
    memset(account, 0, sizeof(Account_t));
    uint32_t search_by;
    char buffer[CHARBUFFER];
    char prompt[BUFFER];
    
    while (1)
    {
        system("clear");
        sprintf(prompt, "Enter %s account ID (or 'r' to return): ", msg);
        
        if (getString(buffer, CHARBUFFER, prompt, false) == INPUT_GO_BACK)
            return INPUT_GO_BACK;
            
        char *endptr;
        search_by = (uint32_t)strtoul(buffer, &endptr, 10);
        
        if (*endptr != '\0' || endptr == buffer) {
            printErrorAndWait("Invalid ID format, please enter a valid positive number");
            continue;
        }
        
        if (search_by == 0) {
            printErrorAndWait("Account ID must be greater than 0");
            continue;
        }
        
        break;
    }
    
    FILE *search_f = fopen(DATA_FILE, "rb");
    if (search_f == NULL)
    {
        printErrorAndWait("Error opening accounts file");
        *found = false;
        return INPUT_ERROR;
    }
    
    Account_t temp_account;
    while (fread(&temp_account, sizeof(Account_t), 1, search_f))
    {
        if (temp_account.id == search_by)
        {
            fclose(search_f);
            *found = true;
            *account = temp_account;
            return INPUT_SUCCESS;
        }
    }
    fclose(search_f);
    *found = false;
    return INPUT_SUCCESS;
}

// Account creation functions
InputStatus_t getPESEL(Account_t *new)
{
    PESEL buffer;
    InputStatus_t status;
    while (1)
    {
        status = getString(buffer, PESEL_LENGTH + 1, "Enter your PESEL (11 digits, or 'r' to return): ", true);
        if (status == INPUT_GO_BACK)
            return INPUT_GO_BACK;
        if (status == INPUT_ERROR)
            return INPUT_ERROR;
            
        if (!checkDigits(buffer)) {
            printErrorAndWait("PESEL must contain only digits (0-9)");
            continue;
        }
        
        if (strlen(buffer) != PESEL_LENGTH) {
            char error_msg[BUFFER];
            sprintf(error_msg, "PESEL must be exactly %d digits long, you entered %zu digits", 
                    PESEL_LENGTH, strlen(buffer));
            printErrorAndWait(error_msg);
            continue;
        }
        
        break;
    }
    strcpy(new->pesel_number, buffer);
    return INPUT_SUCCESS;
}

InputStatus_t getName(Account_t *new)
{
    InputStatus_t status;
    while (1)
    {
        status = getString(new->first_name, CHARBUFFER, "Enter first name (or 'r' to return): ", true);
        if (status == INPUT_GO_BACK)
            return INPUT_GO_BACK;
        if (status == INPUT_ERROR)
            return INPUT_ERROR;
            
        if (!checkLetters(new->first_name)) {
            printErrorAndWait("First name must contain only letters and spaces");
            continue;
        }
        
        if (strlen(new->first_name) == 0) {
            printErrorAndWait("First name cannot be empty");
            continue;
        }
        
        break;
    }
    
    while (1)
    {
        status = getString(new->last_name, CHARBUFFER, "Enter surname (or 'r' to return): ", true);
        if (status == INPUT_GO_BACK)
            return INPUT_GO_BACK;
        if (status == INPUT_ERROR)
            return INPUT_ERROR;
            
        if (!checkLetters(new->last_name)) {
            printErrorAndWait("Last name must contain only letters and spaces");
            continue;
        }
        
        if (strlen(new->last_name) == 0) {
            printErrorAndWait("Last name cannot be empty");
            continue;
        }
        
        break;
    }
    
    return INPUT_SUCCESS;
}

InputStatus_t getLocation(Account_t *new)
{
    InputStatus_t status;
    while (1)
    {
        status = getString(new->address, ADDRBUFFER, "Enter address (or 'r' to return): ", true);
        if (status == INPUT_GO_BACK)
            return INPUT_GO_BACK;
        if (status == INPUT_ERROR)
            return INPUT_ERROR;
            
        if (strlen(new->address) == 0) {
            printErrorAndWait("Address cannot be empty");
            continue;
        }
        
        break;
    }
    return INPUT_SUCCESS;
}

InputStatus_t getBalance(Account_t *new)
{
    return getDouble(&new->balance, CASH_MIN, CASH_MAX, "initial balance");
}

InputStatus_t getDebtInfo(Account_t *new)
{
    return getDouble(&new->debt, 0.0, CASH_MAX, "current debt");
}

void generateIBAN(Account_t *new)
{
    IBAN to_be_generated;
    do
    {
        for (int i = 0; i < IBAN_LENGTH; i++)
        {
            to_be_generated[i] = (rand() % 10) + '0';
        }
        to_be_generated[IBAN_LENGTH] = '\0';
        
    } while (isIBANoverlapping(to_be_generated));
    
    strcpy(new->account_number, to_be_generated);
}

bool isIBANoverlapping(IBAN check_val)
{
    FILE *check_f = fopen(DATA_FILE, "rb");
    if (check_f == NULL)
    {
        return false;
    }
    Account_t check;
    while (fread(&check, sizeof(Account_t), 1, check_f))
    {
        if (strcmp(check.account_number, check_val) == 0)
        {
            fclose(check_f);
            return true;
        }
    }
    fclose(check_f);
    return false;
}

uint32_t getLastID()
{
    FILE *seek_file = fopen(DATA_FILE, "rb");
    if (seek_file == NULL)
    {
        return 0;
    }
    
    Account_t last;
    uint32_t last_id = 0;
    
    while (fread(&last, sizeof(Account_t), 1, seek_file))
    {
        if (last.id > last_id)
        {
            last_id = last.id;
        }
    }
    
    fclose(seek_file);
    return last_id;
}

void createAccount()
{
    Account_t new;
    memset(&new, 0, sizeof(Account_t)); 
    new.id = getLastID() + 1;
    generateIBAN(&new);
    
    if (getName(&new) == INPUT_GO_BACK) return;
    if (getPESEL(&new) == INPUT_GO_BACK) return;
    if (getLocation(&new) == INPUT_GO_BACK) return;
    if (getBalance(&new) == INPUT_GO_BACK) return;
    if (getDebtInfo(&new) == INPUT_GO_BACK) return;
    
    if (confirmation(&new, false))
    {
        FILE *append_file = fopen(DATA_FILE, "ab");
        if (append_file == NULL)
        {
            printf("Error opening file!\n");
            waitingForReturn();
            return;
        }
        if (fwrite(&new, sizeof(Account_t), 1, append_file) != 1)
        {
            printf("Error writing to file!\n");
            fclose(append_file);
            waitingForReturn();
            return;
        }
        fclose(append_file);
        printSuccess();
    }
    else
    {
        printAbort();
    }
}

// Main functions
int getAction()
{
    int key = getchar();
    while (getchar() != '\n')
        ;
    return key;
}

void chooseAction()
{
    printActions();
    int key = getAction();
    switch (key)
    {
    case '1':
        chooseModifyingOperation();
        break;
    case '2':
        chooseDisplayOperation();
        break;
    case '3':
        printHelpMenu();
        break;
    case '4':
        quit_flag = 1;
        break;
    default:
        break;
    }
}

void chooseModifyingOperation()
{
    void (*functionPointer)(void);
    printModifyingOptions();
    int key = getAction();
    switch (key)
    {
    case '1':
        functionPointer = &createAccount;
        break;
    case '2':
        functionPointer = &makeDeposit;
        break;
    case '3':
        functionPointer = &makeWithdrawal;
        break;
    case '4':
        functionPointer = &transferMoney;
        break;
    case '5':
        functionPointer = &takeLoan;
        break;
    case '6':
        functionPointer = &payDebt;
        break;
    default:
        return;
    }
    (*functionPointer)();
}

void chooseDisplayOperation()
{
    void (*functionPointer)(void);
    system("clear");
    printDisplayOptions();
    int key = getAction();

    switch (key)
    {
    case '1':
        functionPointer = &printAllList;
        break;
    case '2':
        functionPointer = &searchList;
        break;
    default:
        return;
    }
    (*functionPointer)();
}

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));  
    
    while (1)
    {
        chooseAction();
        if (quit_flag)
        {
            break;
        }
    }

    return 0;
}

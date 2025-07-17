import time

start = time.time()

month_len = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]


def get_digits(pesel_str): 
    pesel_digits = []
    for digit in pesel_str:
        pesel_digits.append(digit)
    if len(pesel_digits) != 11:
        return False
    else:
        pass

    return True


def verify_digits(pesel_str):   # verifying if the string contains only digits
    for digit in pesel_str:
        if digit.isdigit():
            pass
        else:
            return False
    return True


def is_leapyear(year):

    if year % 400 == 0:
        month_len[1] = 29
    elif year % 100 == 0 and year % 400 != 0:
        month_len[1] = 28
    elif year % 4 == 0:
        month_len[1] = 29
    else:
        month_len[1] = 28


def verify_dob(pesel_digits):   # verification of date of birth

    year = int(pesel_digits[0:2])
    month = int(pesel_digits[2:4])
    day = int(pesel_digits[4:6])
    if (month // 20) == 4:
        year = 1800 + year
    elif (month // 20) == 0:
        year = 1900 + year
    elif (month // 20) == 1:
        year = 2000 + year
    elif (month // 20) == 2:
        year = 2100 + year
    elif (month // 20) == 3:
        year = 2200 + year

    month = month - 20 * (month // 20)

    is_leapyear(year)
    if day < 1 or day > month_len[month-1]:
        return False
    return True



# algorithm for checking control digit, further explanation https://www.gov.pl/web/gov/czym-jest-numer-pesel

def verify_control_digit(pesel_digits):

    sum_of_digits = 0
    iterator = [1, 3, 7, 9, 1, 3, 7, 9, 1, 3]

    sum_of_digits = sum(int(pesel_digits[i]) * iterator[i] for i in range(10)) % 10

    if ((10-sum_of_digits)% 10) != int(pesel_digits[10]):
        return False
    
    return True


invalid_length = invalid_character = invalid_dob = invalid_control_digit = 0
total = correct = man = woman = 0


file = open("1e6.dat", 'r')

for line in file:
    line = line.strip()
    if get_digits(line):
        if verify_digits(line):
            if verify_dob(line):
                if verify_control_digit(line):
                    correct += 1
                    if int(line[9]) % 2 == 0:
                        woman += 1
                    else:
                        man += 1
                else:
                    invalid_control_digit += 1
            else:
                invalid_dob += 1
        else:
            invalid_character += 1
    else:
        invalid_length += 1
    total += 1

file.close()

print(f"Total: {total} Correct: {correct} Men: {man} Women: {woman}")
print(invalid_length, invalid_character, invalid_dob, invalid_control_digit)

print("Runtime [s]= ", time.time()-start)
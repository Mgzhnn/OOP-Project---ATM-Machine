#!/usr/bin/env bash
# Scripted end-to-end tests: builds the ATM with AddressSanitizer and
# UndefinedBehaviorSanitizer, feeds each scenario on stdin, and checks the output.
# Usage, from anywhere:  tests/run_tests.sh      (set CXX to choose the compiler)
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CXX="${CXX:-g++}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

"$CXX" -std=c++20 -Wall -Wextra -pedantic -Werror -g -O1 -fsanitize=address,undefined \
    -fno-omit-frame-pointer -o "$WORK/atm" "$ROOT"/*.cpp || { echo "FAIL: build"; exit 1; }
cp "$ROOT/tests/sample.txt" "$WORK/"
cd "$WORK"

passed=0
failed=0
OUT=""
CODE=0

# run <stdin text>: runs the ATM in $WORK, sets OUT (stdout + stderr) and CODE
run() {
    # Output is capped at 1 MB so a reintroduced input loop fails fast instead of filling memory
    OUT="$(printf '%b' "$1" | timeout 20 ./atm 2>&1 | head -c 1000000; exit "${PIPESTATUS[1]}")"
    CODE=$?
}

check() {  # check <name> <condition result>
    if [ "$2" -eq 0 ] && ! grep -qE "ERROR: AddressSanitizer|runtime error|LeakSanitizer" <<<"$OUT"; then
        passed=$((passed + 1))
    else
        failed=$((failed + 1))
        echo "FAIL: $1 (exit $CODE)"
        tail -5 <<<"$OUT" | sed 's/^/    /'
    fi
}
has() { grep -qF -- "$1" <<<"$OUT"; }
lacks() { ! grep -qF -- "$1" <<<"$OUT"; }

ADMIN='sample.txt\n0\n9999\nadminpw\nno\n'
ALICE="${ADMIN}/UseATM\n0\n11112222\n1234\n"

# --- normal session: withdrawal, account transfer, session summary
run "${ALICE}3\n20000\nno\n4\n2\n300-333\n50000\nno\n5\n/\n/Exit\n"
check "withdraw and transfer" "$(has '2 x 10000 won' && has 'Transfer of 50000 won has completed. Fee charged: 1000 won' \
    && has 'Total Fee: 2000 won' && has 'Final Balance: 428000 won' && has 'Balance: 1250000' && [ $CODE -eq 0 ]; echo $?)"

# --- amounts: negative and overflowing checks are rejected, nothing is deposited
run "${ALICE}2\n2\n-5\n18446744073709551615\ncancel\n1\n5\n/Exit\n"
check "negative and overflowing checks" "$(has 'Invalid input. Try again' && has 'Invalid amount entered.' \
    && has 'No checks deposited' && has '500000 won' && lacks '18446744073709551611'; echo $?)"
run "${ALICE}abc\n1x\n5\n/Exit\n"
check "strict menu numbers" "$(has 'Error! Please choose one operation.' && [ $CODE -eq 0 ]; echo $?)"

# --- end of input at every prompt exits instead of looping
for input in 'sample.txt\n' 'sample.txt\n0\n' "${ALICE}" "${ALICE}2\n" "${ALICE}2\n2\n" \
             "${ALICE}4\n1\n300-333\n5000\n" "${ADMIN}/UseATM\n0\n9999\nadminpw\n"; do
    run "$input"
    check "end of input: $(printf '%b' "$input" | tail -n 2 | tr '\n' '|')" "$(has 'Input closed. Exiting.' && [ $CODE -eq 0 ]; echo $?)"
done

# --- input file validation
bad_file() {  # bad_file <name> <file contents> <expected message>
    printf '%b' "$2" > "$1.txt"
    run "$1.txt\n"
    check "input file: $1" "$(has "$3" && [ $CODE -eq 1 ]; echo $?)"
}
H='2 2 1\nKB\nShinhan\nKB A 100-1 5 111 1\n'
bad_file no_banks      '0 0 1\nKB 123456 Single Unilingual 1 1 1 1\n' 'with at least one bank'
bad_file short_atm     "${H}KB B 100-2 5 222 2\nKB 123456 Single Unilingual 1 1\n" 'ATM #1 must be'
bad_file unknown_bank  "${H}Woori B 100-2 5 222 2\nKB 123456 Single Unilingual 1 1 1 1\n" 'belongs to unknown bank "Woori"'
bad_file unknown_type  "${H}KB B 100-2 5 222 2\nKB 123456 Double Bilingual 1 1 1 1\n" 'unknown type "Double Bilingual"'
bad_file negative_cash "${H}KB B 100-2 5 222 2\nKB 123456 Single Unilingual 1 -1 1 1\n" 'negative bill count'
bad_file dup_card      "${H}Shinhan B 100-2 5 111 2\nKB 123456 Single Unilingual 1 1 1 1\n" 'duplicate card number "111"'
bad_file dup_account   "${H}Shinhan B 100-1 5 222 2\nKB 123456 Single Unilingual 1 1 1 1\n" 'duplicate account number "100-1"'
bad_file bad_serial    "${H}KB B 100-2 5 222 2\nKB 12x456 Single Unilingual 1 1 1 1\n" 'must contain only digits'
bad_file neg_balance   "${H}KB B 100-2 -5 222 2\nKB 123456 Single Unilingual 1 1 1 1\n" 'account #2 (B) has an invalid balance "-5"'

# --- cash transfer: cancelling at either step leaves the ATM's cash unchanged
START_CASH='Remaining cash: 20 x 50,000 won | 40 x 10,000 won | 40 x 5,000 won | 50 x 1,000 won'
run "${ALICE}4\n1\n300-333\n50000\n1 0 0 1\ncancel\n5\n/\n/Exit\n"
check "cash transfer cancelled at amount" "$(has 'Inserted amount does not match' && has 'Transaction cancelled' && has "$START_CASH"; echo $?)"
run "${ALICE}4\n1\n300-333\n50000\n1 0 0 0\ncancel\n5\n/\n/Exit\n"
check "cash transfer cancelled at fee" "$(has 'Transaction cancelled' && has "$START_CASH" && lacks 'Transfer of'; echo $?)"
run "${ALICE}4\n1\n300-333\n50000\n1 0 0 0\n0 0 0 2\nno\n5\n/\n/Exit\n"
check "cash transfer completed" "$(has 'Remaining cash: 21 x 50,000 won | 40 x 10,000 won | 40 x 5,000 won | 52 x 1,000 won' \
    && has 'Balance: 1250000'; echo $?)"

# --- cards, passwords, admin
run "${ADMIN}/UseATM\n0\n22223333\n/UseATM\n1\n1\n22223333\n4321\n5\n/Exit\n"
check "single-bank ATM refuses other banks, multi-bank accepts" "$(has 'only accepts cards from its primary bank' && has 'Welcome, Bob!'; echo $?)"
run "${ADMIN}/UseATM\n0\n11112222\nx\ny\nz\n/Exit\n"
check "three wrong passwords" "$(has 'Too many incorrect password attempts.' && [ "$(grep -c 'Incorrect password.' <<<"$OUT")" -eq 3 ]; echo $?)"
run 'sample.txt\n0\n9999\nadminpw\nyes\n1\n8888\npw2\nno\n/UseATM\n1\n1\n9999\nadminpw\n/UseATM\n1\n1\n8888\npw2\n1\n3\n/Exit\n'
check "admin history; rejected admin card not recorded" "$(has 'Admin account is not allowed on this ATM.' \
    && has 'Admin session (current session)' && lacks 'Card number of user: 9999'; echo $?)"
run 'sample.txt\n0\n\n9999\nadminpw\nno\n/Exit\n'
check "empty admin card rejected" "$(has 'Card number cannot be empty.' && has 'Admin account successfully created!'; echo $?)"

# --- Korean UI: 취소 cancels check entry
run "${ADMIN}/UseATM\n1\n2\n22223333\n4321\n2\n2\n취소\n5\n/Exit\n"
check "Korean cancel" "$(has '수표가 입금되지 않았습니다.'; echo $?)"

echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]

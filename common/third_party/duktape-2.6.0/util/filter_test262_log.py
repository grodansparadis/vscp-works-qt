#!/usr/bin/env python2

import os
import sys
import json
import yaml

def main():
    with open(sys.argv[1], 'rb') as f:
        known_issues = yaml.load(f.read())

    skipstrings = [
        'passed in strict mode',
        'passed in non-strict mode',
        'failed in strict mode as expected',
        'failed in non-strict mode as expected'
    ]

    in_failed_tests = False
    tofix_count = 0     # count of bugs that will be fixed (no uncertainty about proper behavior etc)
    known_errors = []
    diagnosed_errors = []
    unknown_errors = []
    other_errors = []

    for line in sys.stdin:
        if len(line) > 1 and line[-1] == '\n':
            line = line[:-1]

        # Skip success cases

        skip = False
        for sk in skipstrings:
            if sk in line:
                skip = True
        if skip:
            continue

        # Augment error list with "known bugs"

        print(line)  # print error list as is, then refined version later

        if 'failed tests' in line.lower():
            in_failed_tests = True
            continue

        if in_failed_tests and line.strip() == '':
            in_failed_tests = False
            continue

        if in_failed_tests:
            # "  intl402/ch12/12.2/12.2.3_c in non-strict mode"
            tmp = line.strip().split(' ')
            test = tmp[0]

            matched = False
            for kn in known_issues:
                if kn.get('test', None) != test:
                    continue
                if kn.has_key('diagnosed'):
                    tofix_count += 1
                    diagnosed_errors.append(line + '   // diagnosed: ' + kn['diagnosed'])
                elif kn.has_key('knownissue'):
                    # Don't bump tofix_count, as testcase expected result is not certain
                    known_errors.append(line + '   // KNOWN: ' + kn['knownissue'])
                else:
                    tofix_count += 1
                    unknown_errors.append(line + '   // ??? (rule matches)')
                kn['used'] = True  # mark rule used
                matched = True
                break

            if matched:
                continue

            # no match, to fix
            other_errors.append(line)
            tofix_count += 1

    print('')
    print('=== CATEGORISED ERRORS ===')
    print('')

    # With ES2015+ semantic changes to ES5 there are too many known
    # issues to print by default.
    #for i in known_errors:
    #    print(i)

    for i in diagnosed_errors:
        print(i)
    for i in unknown_errors:
        print(i)
    for i in other_errors:
        print(i)

    # Check for unused rules (e.g. bugs fixed)

    print('')
    for kn in known_issues:
        if not kn.has_key('used'):
            print('WARNING: unused rule: ' + json.dumps(kn))

    # Used by testclient

    if len(unknown_errors) > 0 or len(other_errors) > 0:
        print('TEST262 FAILED')
    elif len(known_errors) > 0 or len(diagnosed_errors) > 0:
        # Known and diagnosed errors don't indicate test failure
        # as far as GitHub status is concerned.
        print('TEST262 SUCCESS')
    else:
        print('TEST262 SUCCESS')

    # To fix count

    print('')
    print('KNOWN ISSUE COUNT: ' + str(len(known_errors)))
    print('TO-FIX COUNT: ' + str(tofix_count))
    print('  = test case failures which need fixing (Duktape bugs, uninvestigated)')


if __name__ == '__main__':
    main()

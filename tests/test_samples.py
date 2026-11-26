"""Exact-output regression cases; run with the compiled executable path."""
import subprocess
import sys

CASES = [
    ("-1+x+x^2+3x^3+sinx+cos^2x+sin^4x-5cos^8x",
     "3x^3+x^2+x+sin^4x+sinx-5cos^8x+cos^2x-1",
     "9x^2+2x+4sin^3xcosx+40sinxcos^7x-2sinxcosx+cosx+1"),
    ("-xsinxcos^2x*x^3sinx", "-x^4sin^2xcos^2x",
     "2x^4sin^3xcosx-2x^4sinxcos^3x-4x^3sin^2xcos^2x"),
    ("1/(1-1/(1-sinx+x))", "(x-sinx+1)/(x-sinx)",
     "(cosx-1)/(x^2-2xsinx+sin^2x)"),
    ("(1+x)/x^2+1/x", "(2x^2+x)/x^3", "(-2x^4-2x^3)/x^6"),
    ("(1/x)/(1+x)", "1/(x^2+x)", "(-2x-1)/(x^4+2x^3+x^2)"),
    ("1/2x", "1/2x", "-2/4x^2"),
    ("9/(-6x)", "9/-6x", "54/36x^2"),
    ("x/x", "x/x", "0"),
    ("(1/x-1/x)+1", "x^2/x^2", "0"),
    ("1/x/x", "1/x^2", "-2x/x^4"),
    ("x-(x-1)*2", "-x+2", "-1"),
    ("sin^2x+cos^2x", "sin^2x+cos^2x", "0"),
    ("-(-x)", "x", "1"),
    ("0", "0", "0"),
]

executable = sys.argv[1] if len(sys.argv) > 1 else "./build/code"
for expression, value, derivative in CASES:
    result = subprocess.run([executable], input=expression + "\n", text=True,
                            capture_output=True, check=True)
    assert result.stdout == value + "\n" + derivative + "\n", (expression, result.stdout)
print(f"Passed {len(CASES)} exact-output cases.")

Import("env")
import random

print("""
=========================================================
Enter Board child ID
=========================================================""")
child_id = input()
token_id = random.randint(0, 0xFFFF)
env.Append(
    CPPDEFINES=[("BUILD_UNIQU_TOKEN",  token_id),("CAN_CHILD_ID",  child_id)],
)

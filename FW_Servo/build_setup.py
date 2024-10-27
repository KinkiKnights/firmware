Import("env")

print("""
=========================================================
Enter Board child ID
=========================================================""")
child_id = input()
env.Append(
    CPPDEFINES=[("CAN_CHILD_ID",  child_id)],
)

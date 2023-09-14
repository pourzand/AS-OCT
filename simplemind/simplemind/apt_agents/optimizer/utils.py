import os

def _gen_ref(case):
    ref = None
    if case["dataset"]=="lidc":
        ref = os.path.join(case["reference"], case["id"])
        if not os.path.exists(ref):
            raise IOError("Cannot find %s" % ref)
    elif case["dataset"]=="cxr":
        ref = os.path.join(case["reference"], case["id"])
        if not os.path.exists(ref):
            raise IOError("Cannot find %s" % ref)
    else:
        if os.path.exists(case["reference"]):
            ref = case["reference"]
    return ref

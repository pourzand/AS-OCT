import traceback

class NonZeroExitCode(Exception):
    pass

class TimedOut(Exception):
    pass
    
class JobStopped(Exception):
    pass
    
class NotReady(Exception):
    pass

class ErrorValue:
    def __init__(self, exc_info=None, format_exc=None):
        self.exc_info = exc_info
        self.format_exc = format_exc
        
    def what(self):
        if self.exc_info is None:
            print("ErrorValue cause: Not provided!")
        else:
            print("ErrorValue cause:")
            for i in traceback.format_exception_only(exc_info[0], exc_info[1]):
                print(i.strip())
                
    def __str__(self):
        if self.exc_info is None:
            return "ErrorValue cause: Not provided!"
        else:
            msg = []
            msg.append("ErrorValue cause:")
            print(self.exc_info)
            print(self.format_exc)
            print("^^^")
            for i in traceback.format_exception_only(self.exc_info[0], self.exc_info[1]):
                msg.append(i.strip())
            print(">>>>")
            return "\n".join(msg)
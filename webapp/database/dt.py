from datetime import date, time, datetime
from dateutil import tz

now = datetime.now(tz=tz.UTC)

now_str = str(now)


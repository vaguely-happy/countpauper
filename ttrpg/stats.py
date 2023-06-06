from typing import Callable

class Property(object):
    def __init__(self, name):
        self.name =  name

    def __get__(self, instance, owner):
        return instance.stats.get(self.name)

    def __set__(self, instance, value):
        instance.stats[self.name] = value

    def __delete__(self, instance):
        instance.stats.pop(self.name)

class Stat(Property):
    def __init__(self, name, minimum=0):
        super(Stat,self).__init__(name)
        self.minimum = minimum

    def __get__(self, instance, owner):
        boni = instance.get_boni(self.name)
        return max(self.minimum, instance.stats.get(self.name) + sum(boni.values()))

    def __set__(self, instance, value):
        instance.stats[self.name] = value

    def __delete__(self, instance):
        instance.stats.pop(self.name)

class Identifier(Property):
    def __init__(self, name):
        super(Identifier, self).__init__(name)

class Ability(Stat):
    def __init__(self, name):
        super(Ability,self).__init__(name)

class Counter(object):
    def __init__(self, a, b=None):
        if b is None:
            self._maximum = a
            self._value = self.maximum() or 0
        else:
            self._maximum = b
            self._value = a

    def value(self):
        if (m:= self.maximum()) is None:
            return max(0, self._value)
        else:
            return max(0, min(self._value, m))

    def maximum(self):
        if self._maximum is None:
            return None
        elif isinstance(self._maximum, Callable):
            return self._maximum()
        else:
            return self._maximum

    def __add__(self, increase):
        result = Counter(self.value() + increase, self._maximum)
        return result

    def __sub__(self, decrease):
        result = Counter(self.value() - decrease, self._maximum)
        return result

    def __str__(self):
        return f'{self.value()}/{self.maximum()}'

    def __int__(self):
        return self.value()

    def __conform__(self, protocol):
        return self.value()

    def __bool__(self):
        return self.value() > 0

    def __eq__(self, other):
        if isinstance(other,  Counter):
            return self.value() == other.value() and self.maximum() == other.maximum()
        else:
            return self.value() == other

    def __lt__(self, other):
        return self.value() < other

    def __ge__(self, other):
        return self.value() >= other

    def reset(self):
        self._value = self.maximum()

class CounterStat(Stat):
    def __init__(self, name):
        super(CounterStat, self).__init__(name)


    def __get__(self, instance, owner):
        stat = instance.stats.get(self.name)
        boni = instance.get_boni(self.name)
        bonus_value = sum(boni.values())
        return stat + bonus_value

    def __set__(self, instance, value):
        if isinstance(value, Counter):
            instance.stats[self.name] = value
        else:
            stat = instance.stats.get(self.name)
            instance.stats[self.name] = stat + (value - stat.value())
        return instance.stats[self.name]


# exceptions
from space.time import Period
import space.constants as constants
from specification import Resource

class ItemException(Exception):
    pass

class StackException(ItemException):
    pass

# private abstract base types
class _Item(Resource):
    def __init__(self, **kwargs):
        super(_Item,self).__init__(kwargs['specification'])

    def volume(self):
        return self.amount * self.specification.volume

    def consume(self, _):
        raise ItemException("Can't consume {}", self)

    def __repr__(self):
        return "{} x {}".format(self.amount, self.specification.name)

class _Unique(object):
    def __init__(self,**kwargs):
        self.amount = 1

class _Stackable(object):
    def __init__(self,**kwargs):
        self.amount = kwargs['amount']

    def split(self, amount):
        if self.amount < amount:
            raise StackException("Insufficient amount", self, amount)
        self.amount -= amount
        return self.specification.create(amount=amount)

    def stack(self, item):
        if self.specification != item.specification:
            raise StackException("Incompatible types", self, item)
        self.amount += item.amount
        item.amount = 0

class _Consumable(object):
    def __init__(self, **kwargs):
        pass

    def consume(self, _):
        self.amount = 0

# major category Goods
class Good(_Item, _Stackable):
    def __init__(self, **kwargs):
        _Item.__init__(self, **kwargs)
        _Stackable.__init__(self, **kwargs)

# category base classes
class _Supply(Good):
    def __init__(self, **kwargs):
        super(_Supply, self).__init__(**kwargs)

class _Material(Good):
    def __init__(self, **kwargs):
        super(_Material, self).__init__(**kwargs)

# Public types, referenced from specification
class Food(_Supply, _Consumable):
    def __init__(self, **kwargs):
        _Supply.__init__(self,**kwargs)
        _Consumable.__init__(self, **kwargs)

    def consume(self, user):
        user.hunger -= constants.calories_per_meal
        _Consumable.consume(self, user)

class Grain(_Material):
    def __init__(self, **kwargs):
        _Material.__init__(self, **kwargs)

class Water(_Material):
    def __init__(self, **kwargs):
        _Material.__init__(self, **kwargs)

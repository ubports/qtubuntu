TEMPLATE = subdirs

SUBDIRS += common ubuntumirclient ubuntuthemeplugin

ubuntumirclient.depends = common
ubuntuthemeplugin.depends = common

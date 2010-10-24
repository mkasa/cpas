
# -*- python -*-
APPNAME = 'cpas'
VERSION = '1.02'

def set_options(ctx):
    ctx.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')

def build(bld):
    bld(features='cc cprogram',
        source='cpas.c',
        target='cpas')


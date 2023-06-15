.. 
   Input file: fe/ips/riscv_gwt/doc/NONSECURED_RI5CY_DEBUG_reference.md

Register map
^^^^^^^^^^^^


Overview
""""""""


Refer to :ref:`GAP9 address map<REF_MEMORY_MAP_DETAIL>` for the base address to be used.

.. table:: 
    :align: center
    :widths: 40 12 12 90

    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |                          Name                          |Offset|Width|                            Description                            |
    +========================================================+======+=====+===================================================================+
    |:ref:`CTRL<non_secured_riscv_debug__CTRL>`              |     0|   32|Debug control configuration register.                              |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`HIT<non_secured_riscv_debug__HIT>`                |     4|   32|Debug hit status register.                                         |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`IE<non_secured_riscv_debug__IE>`                  |     8|   32|Debug exception trap enable configuration register.                |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CAUSE<non_secured_riscv_debug__CAUSE>`            |    12|   32|Debug trap cause status register.                                  |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`NPC<non_secured_riscv_debug__NPC>`                |  8192|   32|Debug next program counter value register.                         |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`PPC<non_secured_riscv_debug__PPC>`                |  8196|   32|Debug previous program counter value register.                     |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR0<non_secured_riscv_debug__GPR0>`              |  1024|   32|Core general purpose register 0 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR1<non_secured_riscv_debug__GPR1>`              |  1028|   32|Core general purpose register 1 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR2<non_secured_riscv_debug__GPR2>`              |  1032|   32|Core general purpose register 2 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR3<non_secured_riscv_debug__GPR3>`              |  1036|   32|Core general purpose register 3 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR4<non_secured_riscv_debug__GPR4>`              |  1040|   32|Core general purpose register 4 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR5<non_secured_riscv_debug__GPR5>`              |  1044|   32|Core general purpose register 5 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR6<non_secured_riscv_debug__GPR6>`              |  1048|   32|Core general purpose register 6 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR7<non_secured_riscv_debug__GPR7>`              |  1052|   32|Core general purpose register 7 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR8<non_secured_riscv_debug__GPR8>`              |  1056|   32|Core general purpose register 8 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR9<non_secured_riscv_debug__GPR9>`              |  1060|   32|Core general purpose register 9 value register.                    |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR10<non_secured_riscv_debug__GPR10>`            |  1064|   32|Core general purpose register 10 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR11<non_secured_riscv_debug__GPR11>`            |  1068|   32|Core general purpose register 11 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR12<non_secured_riscv_debug__GPR12>`            |  1072|   32|Core general purpose register 12 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR13<non_secured_riscv_debug__GPR13>`            |  1076|   32|Core general purpose register 13 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR14<non_secured_riscv_debug__GPR14>`            |  1080|   32|Core general purpose register 14 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR15<non_secured_riscv_debug__GPR15>`            |  1084|   32|Core general purpose register 15 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR16<non_secured_riscv_debug__GPR16>`            |  1088|   32|Core general purpose register 16 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR17<non_secured_riscv_debug__GPR17>`            |  1092|   32|Core general purpose register 17 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR18<non_secured_riscv_debug__GPR18>`            |  1096|   32|Core general purpose register 18 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR19<non_secured_riscv_debug__GPR19>`            |  1100|   32|Core general purpose register 19 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR20<non_secured_riscv_debug__GPR20>`            |  1104|   32|Core general purpose register 20 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR21<non_secured_riscv_debug__GPR21>`            |  1108|   32|Core general purpose register 21 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR22<non_secured_riscv_debug__GPR22>`            |  1112|   32|Core general purpose register 22 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR23<non_secured_riscv_debug__GPR23>`            |  1116|   32|Core general purpose register 23 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR24<non_secured_riscv_debug__GPR24>`            |  1120|   32|Core general purpose register 24 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR25<non_secured_riscv_debug__GPR25>`            |  1124|   32|Core general purpose register 25 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR26<non_secured_riscv_debug__GPR26>`            |  1128|   32|Core general purpose register 26 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR27<non_secured_riscv_debug__GPR27>`            |  1132|   32|Core general purpose register 27 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR28<non_secured_riscv_debug__GPR28>`            |  1136|   32|Core general purpose register 28 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR29<non_secured_riscv_debug__GPR29>`            |  1140|   32|Core general purpose register 29 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR30<non_secured_riscv_debug__GPR30>`            |  1144|   32|Core general purpose register 30 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`GPR31<non_secured_riscv_debug__GPR31>`            |  1148|   32|Core general purpose register 31 value register.                   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_MSTATUS<non_secured_riscv_debug__CSR_MSTATUS>`| 19456|   32|Core CSR machine status value register.                            |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_MTVEC<non_secured_riscv_debug__CSR_MTVEC>`    | 19476|   32|Core CSR machine vector-trap base address value register.          |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_MEPC<non_secured_riscv_debug__CSR_MEPC>`      | 19716|   32|Core CSR machine exception program counter value register.         |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_MCAUSE<non_secured_riscv_debug__CSR_MCAUSE>`  | 19720|   32|Core CSR machine trap cause value register.                        |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_PCCR<non_secured_riscv_debug__CSR_PCCR>`      | 24064|   32|Core CSR performance counter counter register.                     |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_PCER<non_secured_riscv_debug__CSR_PCER>`      | 24192|   32|Core CSR performance counter enable configuration register.        |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_PCMR<non_secured_riscv_debug__CSR_PCMR>`      | 24196|   32|Core CSR performance counter mode configuration register.          |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_HWLP0S<non_secured_riscv_debug__CSR_HWLP0S>`  | 24256|   32|Core CSR hardware loop 0 start configuration register.             |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_HWLP0E<non_secured_riscv_debug__CSR_HWLP0E>`  | 24260|   32|Core CSR hardware loop 0 end configuration register.               |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_HWLP0C<non_secured_riscv_debug__CSR_HWLP0C>`  | 24264|   32|Core CSR hardware loop 0 counter configuration register.           |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_HWLP1S<non_secured_riscv_debug__CSR_HWLP1S>`  | 24272|   32|Core CSR hardware loop 1 start configuration register.             |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_HWLP1E<non_secured_riscv_debug__CSR_HWLP1E>`  | 24276|   32|Core CSR hardware loop 1 end configuration register.               |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_HWLP1C<non_secured_riscv_debug__CSR_HWLP1C>`  | 24280|   32|Core CSR hardware loop 1 counter configuration register.           |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_PRIVLV<non_secured_riscv_debug__CSR_PRIVLV>`  | 28736|   32|Cose CSR privilege level status register.                          |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_UHARTID<non_secured_riscv_debug__CSR_UHARTID>`| 16464|   32|Core CSR user privilege mode hardware thread ID status register.   |
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+
    |:ref:`CSR_MHARTID<non_secured_riscv_debug__CSR_MHARTID>`| 31824|   32|Core CSR machine privilege mode hardware thread ID status register.|
    +--------------------------------------------------------+------+-----+-------------------------------------------------------------------+

.. _non_secured_riscv_debug__CTRL:

CTRL
""""

Debug control configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__HIT:

HIT
"""

Debug hit status register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__IE:

IE
""

Debug exception trap enable configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CAUSE:

CAUSE
"""""

Debug trap cause status register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__NPC:

NPC
"""

Debug next program counter value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__PPC:

PPC
"""

Debug previous program counter value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR0:

GPR0
""""

Core general purpose register 0 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR1:

GPR1
""""

Core general purpose register 1 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR2:

GPR2
""""

Core general purpose register 2 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR3:

GPR3
""""

Core general purpose register 3 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR4:

GPR4
""""

Core general purpose register 4 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR5:

GPR5
""""

Core general purpose register 5 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR6:

GPR6
""""

Core general purpose register 6 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR7:

GPR7
""""

Core general purpose register 7 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR8:

GPR8
""""

Core general purpose register 8 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR9:

GPR9
""""

Core general purpose register 9 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR10:

GPR10
"""""

Core general purpose register 10 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR11:

GPR11
"""""

Core general purpose register 11 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR12:

GPR12
"""""

Core general purpose register 12 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR13:

GPR13
"""""

Core general purpose register 13 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR14:

GPR14
"""""

Core general purpose register 14 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR15:

GPR15
"""""

Core general purpose register 15 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR16:

GPR16
"""""

Core general purpose register 16 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR17:

GPR17
"""""

Core general purpose register 17 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR18:

GPR18
"""""

Core general purpose register 18 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR19:

GPR19
"""""

Core general purpose register 19 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR20:

GPR20
"""""

Core general purpose register 20 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR21:

GPR21
"""""

Core general purpose register 21 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR22:

GPR22
"""""

Core general purpose register 22 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR23:

GPR23
"""""

Core general purpose register 23 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR24:

GPR24
"""""

Core general purpose register 24 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR25:

GPR25
"""""

Core general purpose register 25 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR26:

GPR26
"""""

Core general purpose register 26 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR27:

GPR27
"""""

Core general purpose register 27 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR28:

GPR28
"""""

Core general purpose register 28 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR29:

GPR29
"""""

Core general purpose register 29 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR30:

GPR30
"""""

Core general purpose register 30 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__GPR31:

GPR31
"""""

Core general purpose register 31 value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_MSTATUS:

CSR_MSTATUS
"""""""""""

Core CSR machine status value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_MTVEC:

CSR_MTVEC
"""""""""

Core CSR machine vector-trap base address value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_MEPC:

CSR_MEPC
""""""""

Core CSR machine exception program counter value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_MCAUSE:

CSR_MCAUSE
""""""""""

Core CSR machine trap cause value register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_PCCR:

CSR_PCCR
""""""""

Core CSR performance counter counter register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_PCER:

CSR_PCER
""""""""

Core CSR performance counter enable configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_PCMR:

CSR_PCMR
""""""""

Core CSR performance counter mode configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_HWLP0S:

CSR_HWLP0S
""""""""""

Core CSR hardware loop 0 start configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_HWLP0E:

CSR_HWLP0E
""""""""""

Core CSR hardware loop 0 end configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_HWLP0C:

CSR_HWLP0C
""""""""""

Core CSR hardware loop 0 counter configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_HWLP1S:

CSR_HWLP1S
""""""""""

Core CSR hardware loop 1 start configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_HWLP1E:

CSR_HWLP1E
""""""""""

Core CSR hardware loop 1 end configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_HWLP1C:

CSR_HWLP1C
""""""""""

Core CSR hardware loop 1 counter configuration register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_PRIVLV:

CSR_PRIVLV
""""""""""

Cose CSR privilege level status register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_UHARTID:

CSR_UHARTID
"""""""""""

Core CSR user privilege mode hardware thread ID status register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

.. _non_secured_riscv_debug__CSR_MHARTID:

CSR_MHARTID
"""""""""""

Core CSR machine privilege mode hardware thread ID status register.

.. table:: 
    :align: center
    :widths: 13 12 45 24 85

    +----+---+----+-----+-----------+
    |Bit#|R/W|Name|Reset|Description|
    +====+===+====+=====+===========+
    +----+---+----+-----+-----------+

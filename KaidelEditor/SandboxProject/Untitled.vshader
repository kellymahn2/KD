MainNodePos: [1156.8772, 57.8192444]
Nodes:
  - Type: FloatFunc
    NodeID: 8
    Position: [167.171082, 344.597107]
    Func: Sin
    Value: 0
  - Type: Vec4Constant
    NodeID: 3
    Position: [-69.2772217, -147.633179]
    Vec4: [0, 0, 0, 0]
    ParameterName: IfTrue
  - Type: FloatOp
    NodeID: 11
    Position: [451.480225, 342.661621]
    OpType: Multiply
    ValueA: 0
    ValueB: 0.5
  - Type: Vec4Constant
    NodeID: 4
    Position: [-69.2772217, 64.3668213]
    Vec4: [0, 0, 0, 0]
    ParameterName: IfFalse
  - Type: Input
    NodeID: 6
    Position: [-40.6343994, 349.112854]
    IType: Time
  - Type: Easing
    NodeID: 7
    Position: [864.059937, 48.1685791]
    Ease: Bounce
    VType: Vec4
  - Type: FloatOp
    NodeID: 9
    Position: [685.93689, 334.172424]
    OpType: Add
    ValueA: 0
    ValueB: 0.5
Connections:
  - From:
      Node: 3
      Port: 0
    To:
      Node: 7
      Port: 0
  - From:
      Node: 4
      Port: 0
    To:
      Node: 7
      Port: 1
  - From:
      Node: 6
      Port: 0
    To:
      Node: 8
      Port: 0
  - From:
      Node: 7
      Port: 0
    To:
      Node: 0
      Port: 0
  - From:
      Node: 9
      Port: 0
    To:
      Node: 7
      Port: 2
  - From:
      Node: 11
      Port: 0
    To:
      Node: 9
      Port: 0
  - From:
      Node: 8
      Port: 0
    To:
      Node: 11
      Port: 0
DefaultTextures:
  {}
package imp.verification;

import com.microsoft.z3.BoolExpr;
import com.microsoft.z3.Context;
import com.microsoft.z3.Solver;
import com.microsoft.z3.Status;

import imp.ast.method.MethodDeclaration;
import imp.ast.condition.ConditionClause;
import imp.ast.condition.ConditionList;
import imp.ast.condition.EnsuresClause;
import imp.ast.condition.RequiresClause;
import imp.ast.statement.BlockStatement;
import imp.ast.statement.Statement;

import java.util.Optional;

public class Method {
    private static Method instance;
    
    private Method() {
        }
    public boolean Verify(Context ctx, MethodDeclaration method) {
        ConditionList conditionals = method.conditionList();

        // Format conditions list to P and Q
        BoolExpr P = conditionals.requiresClauses().stream()
                .map(condition -> condition.interpret(ctx))
                .reduce(ctx.mkTrue(), ctx::mkAnd);

        BoolExpr Q = conditionals.ensuresClauses().stream()
                .map(condition -> condition.interpret(ctx))
                .reduce(ctx.mkTrue(), ctx::mkAnd);

        BlockStatement statements = new BlockStatement(method.methodBody().statements());
        BoolExpr  avc = AVC.getInstance().avc(ctx, statements, Q);
        BoolExpr awp = AWP.getInstance().awp(ctx, statements, Q);

        Solver solver = ctx.mkSolver();
        solver.add(ctx.mkNot(ctx.mkAnd(ctx.mkImplies(P, awp), avc)));
        
        return solver.check() == Status.UNSATISFIABLE;
    }

    public static Method getInstance() {
        if (instance == null) {
            instance = new Method();
        }
        return instance;
    }
}
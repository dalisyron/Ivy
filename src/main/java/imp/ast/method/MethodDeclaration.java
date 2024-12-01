
package imp.ast.method;

import imp.ast.ASTNode;
import imp.ast.condition.ConditionList;
import imp.ast.statement.Statement;
import imp.ast.variable.Identifier;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

public record MethodDeclaration(
        Identifier name,
        Optional<ParameterList> parameterList,
        Optional<ReturnValue> returnValue,
        Optional<ConditionList> conditionList,
        MethodBody methodBody
) implements ASTNode {

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        // Append the method signature
        sb.append("method ").append(name);

        // Ensure brackets for the parameter list are always present
        if (parameterList.isPresent()) {
            sb.append(parameterList.get());
        } else {
            sb.append("()");
        }

        // Add return value if present
        returnValue.ifPresent(value -> sb.append(" returns (").append(value).append(")"));

        // Add conditions (e.g., requires/ensures) on new lines
        conditionList.ifPresent(conditions -> sb.append(System.lineSeparator()).append(conditions));

        // Append the method body
        sb.append(" {").append(System.lineSeparator());
        sb.append(methodBody).append(System.lineSeparator());
        sb.append("}");

        return sb.toString();
    }

    @Override
    public List<ASTNode> getChildren() {
        ArrayList<ASTNode> children = new ArrayList<>();

        children.add(name);

        parameterList.ifPresent(children::add);

        returnValue.ifPresent(children::add);

        conditionList.ifPresent(children::add);

        children.add(methodBody);

        return children;
    }
}